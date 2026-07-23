#include "Synergy/PBSynergyDataSyncCommandlet.h"

#include "Engine/DataTable.h"
#include "GoogleSheetConfig.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include "Misc/PackageName.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"
#include "UObject/SavePackage.h"
#include "UObject/UObjectIterator.h"

namespace
{
	constexpr double SheetFetchTimeoutSeconds = 60.0;
	constexpr const TCHAR* SynergyLoaderPath = TEXT("/Game/Data/Loaders/Synergy/GSC_Synergy");
	constexpr const TCHAR* SynergyTierLoaderPath = TEXT("/Game/Data/Loaders/Synergy/GSC_SynergyTier");
	constexpr const TCHAR* SynergyTablePath = TEXT("/Game/Data/Tables/Synergy/DT_Synergy");
	constexpr const TCHAR* SynergyTierTablePath = TEXT("/Game/Data/Tables/Synergy/DT_SynergyTier");

	FString MakeObjectPath(const FString& PackagePath)
	{
		return FString::Printf(
			TEXT("%s.%s"),
			*PackagePath,
			*FPackageName::GetLongPackageAssetName(PackagePath));
	}

	UObject* LoadAsset(const FString& PackagePath)
	{
		return StaticLoadObject(
			UObject::StaticClass(),
			nullptr,
			*MakeObjectPath(PackagePath),
			nullptr,
			LOAD_NoWarn);
	}

	UGoogleSheetConfig* ConfigureLoader(const TCHAR* PackagePath, const TCHAR* RangeTo)
	{
		UGoogleSheetConfig* Config = Cast<UGoogleSheetConfig>(LoadAsset(PackagePath));
		if (!IsValid(Config) || !IsValid(Config->DataParser))
		{
			UE_LOG(LogTemp, Error, TEXT("[SynergySync] Invalid loader: %s"), PackagePath);
			return nullptr;
		}

		Config->Modify();
		Config->RangeFrom = TEXT("A1");
		Config->RangeTo = RangeTo;
		Config->bAutoSaveOnComplete = false;
		Config->MarkPackageDirty();
		return Config;
	}

	bool FetchAndWait(UGoogleSheetConfig* Config)
	{
		if (!IsValid(Config))
		{
			return false;
		}

		Config->Fetch();
		const double Deadline = FPlatformTime::Seconds() + SheetFetchTimeoutSeconds;
		while (Config->FetchStatus == EFetchStatus::Loading
			&& FPlatformTime::Seconds() < Deadline)
		{
			FHttpModule::Get().GetHttpManager().Tick(0.05f);
			FPlatformProcess::Sleep(0.01f);
		}

		if (Config->FetchStatus != EFetchStatus::Success)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[SynergySync] Fetch failed. Loader=%s Status=%d Message=%s"),
				*GetNameSafe(Config),
				static_cast<int32>(Config->FetchStatus),
				*Config->LastMessage);
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("[SynergySync] Fetch succeeded: %s"), *GetNameSafe(Config));
		return true;
	}

	template <typename RowType>
	const UDataTable* LoadTypedTable(const TCHAR* PackagePath, const TCHAR* Label)
	{
		const UDataTable* Table = Cast<UDataTable>(LoadAsset(PackagePath));
		if (!IsValid(Table) || Table->GetRowStruct() != RowType::StaticStruct())
		{
			UE_LOG(LogTemp, Error, TEXT("[SynergySync] Invalid %s table: %s"), Label, PackagePath);
			return nullptr;
		}
		return Table;
	}

	bool ValidateSynergyTables()
	{
		const UDataTable* SynergyTable = LoadTypedTable<FPBSynergyTableRow>(
			SynergyTablePath,
			TEXT("Synergy"));
		const UDataTable* TierTable = LoadTypedTable<FPBSynergyTierRow>(
			SynergyTierTablePath,
			TEXT("SynergyTier"));
		if (!SynergyTable || !TierTable)
		{
			return false;
		}

		bool bValid = !SynergyTable->GetRowMap().IsEmpty() && !TierTable->GetRowMap().IsEmpty();
		TSet<FName> SynergyIds;
		for (const FName RowName : SynergyTable->GetRowNames())
		{
			SynergyIds.Add(RowName);
			const FPBSynergyTableRow* Row = SynergyTable->FindRow<FPBSynergyTableRow>(
				RowName,
				TEXT("SynergySync"),
				false);
			if (!Row || Row->DisplayName.IsEmpty() || Row->DescriptionKey.IsNone())
			{
				UE_LOG(LogTemp, Error, TEXT("[SynergySync] Incomplete Synergy row: %s"), *RowName.ToString());
				bValid = false;
			}
		}

		TMap<FName, TSet<int32>> RequiredCountsBySynergy;
		for (const FName RowName : TierTable->GetRowNames())
		{
			const FPBSynergyTierRow* Row = TierTable->FindRow<FPBSynergyTierRow>(
				RowName,
				TEXT("SynergySync"),
				false);
			if (!Row || !SynergyIds.Contains(Row->SynergyId) || Row->RequiredCount <= 0
				|| Row->EffectSetId.IsNone() || Row->TierDescriptionKey.IsNone())
			{
				UE_LOG(LogTemp, Error, TEXT("[SynergySync] Invalid SynergyTier row: %s"), *RowName.ToString());
				bValid = false;
				continue;
			}

			TSet<int32>& RequiredCounts = RequiredCountsBySynergy.FindOrAdd(Row->SynergyId);
			if (RequiredCounts.Contains(Row->RequiredCount))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[SynergySync] Duplicate tier count. Synergy=%s Count=%d"),
					*Row->SynergyId.ToString(),
					Row->RequiredCount);
				bValid = false;
			}
			RequiredCounts.Add(Row->RequiredCount);
		}

		UE_LOG(LogTemp, Display,
			TEXT("[SynergySync] Validation rows. Synergy=%d Tier=%d Result=%s"),
			SynergyTable->GetRowMap().Num(),
			TierTable->GetRowMap().Num(),
			bValid ? TEXT("Success") : TEXT("Failure"));
		return bValid;
	}

	bool IsApprovedPackage(const FString& PackageName)
	{
		return PackageName.StartsWith(TEXT("/Game/Data/Loaders/Synergy/"))
			|| PackageName == SynergyTablePath
			|| PackageName == SynergyTierTablePath
			|| PackageName.StartsWith(TEXT("/Game/Data/DataAssets/Synergy/"));
	}

	bool SaveApprovedDirtyPackages()
	{
		TArray<UPackage*> PackagesToSave;
		for (TObjectIterator<UPackage> It; It; ++It)
		{
			UPackage* Package = *It;
			if (IsValid(Package) && Package->IsDirty() && IsApprovedPackage(Package->GetName()))
			{
				PackagesToSave.Add(Package);
			}
		}

		PackagesToSave.Sort([](const UPackage& Left, const UPackage& Right)
		{
			return Left.GetName() < Right.GetName();
		});

		for (UPackage* Package : PackagesToSave)
		{
			const FString PackageName = Package->GetName();
			const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
			UObject* Asset = FindObject<UObject>(Package, *AssetName);
			if (!IsValid(Asset))
			{
				UE_LOG(LogTemp, Error, TEXT("[SynergySync] Missing top-level asset: %s"), *PackageName);
				return false;
			}

			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;
			const FString Filename = FPackageName::LongPackageNameToFilename(
				PackageName,
				FPackageName::GetAssetPackageExtension());
			if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
			{
				UE_LOG(LogTemp, Error, TEXT("[SynergySync] Failed to save package: %s"), *PackageName);
				return false;
			}
		}

		UE_LOG(LogTemp, Display, TEXT("[SynergySync] Saved %d packages."), PackagesToSave.Num());
		return true;
	}
}

UPBSynergyDataSyncCommandlet::UPBSynergyDataSyncCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UPBSynergyDataSyncCommandlet::Main(const FString& Params)
{
	static_cast<void>(Params);

	UGoogleSheetConfig* SynergyConfig = ConfigureLoader(SynergyLoaderPath, TEXT("F1000"));
	UGoogleSheetConfig* TierConfig = ConfigureLoader(SynergyTierLoaderPath, TEXT("E1000"));
	if (!SynergyConfig || !TierConfig
		|| !FetchAndWait(SynergyConfig)
		|| !FetchAndWait(TierConfig)
		|| !ValidateSynergyTables()
		|| !SaveApprovedDirtyPackages())
	{
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[SynergySync] Synergy data synchronization completed."));
	return 0;
}
