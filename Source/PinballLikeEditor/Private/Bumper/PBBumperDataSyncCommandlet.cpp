#include "Bumper/PBBumperDataSyncCommandlet.h"

#include "AssetToolsModule.h"
#include "BlueprintEditorLibrary.h"
#include "Bumper/PBBumperTableParser.h"
#include "Bumper/PBBumperTriggerTableParser.h"
#include "Bumper/PBBumperEffectTableParser.h"
#include "Engine/Blueprint.h"
#include "Engine/DataTable.h"
#include "GoogleSheetConfig.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include "IAssetTools.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBloodOverdriveBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossDamageBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossVulnerabilityBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBComboArcBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBComboBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBCounterShieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBGateSupportFieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBKineticShellBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"
#include "UObject/SavePackage.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr double SheetFetchTimeoutSeconds = 60.0;
	namespace DataPath
	{
		constexpr const TCHAR* CollectionTable = TEXT("/Game/Data/Tables/DT_Collection");
		constexpr const TCHAR* BumperTable = TEXT("/Game/Data/Tables/Bumper/DT_Bumper");
		constexpr const TCHAR* BumperTriggerTable = TEXT("/Game/Data/Tables/Bumper/DT_BumperTrigger");
		constexpr const TCHAR* BumperEffectTable = TEXT("/Game/Data/Tables/Bumper/DT_BumperEffect");
		constexpr const TCHAR* SharedEffectTable = TEXT("/Game/Data/Tables/Effect/DT_Effect");
		constexpr const TCHAR* SharedEffectParamTable = TEXT("/Game/Data/Tables/Effect/DT_EffectParam");

		constexpr const TCHAR* BumperLoader = TEXT("/Game/Data/Loaders/Bumper/GSC_Bumper");
		constexpr const TCHAR* BumperTriggerLoader = TEXT("/Game/Data/Loaders/Bumper/GSC_BumperTrigger");
		constexpr const TCHAR* BumperEffectLoader = TEXT("/Game/Data/Loaders/Bumper/GSC_BumperEffect");
		constexpr const TCHAR* SharedEffectLoader = TEXT("/Game/Data/Loaders/Effect/GSC_Effect");
		constexpr const TCHAR* SharedEffectParamLoader = TEXT("/Game/Data/Loaders/Effect/GSC_EffectParam");
	}

	struct FApprovedAssetRename
	{
		const TCHAR* SourcePackage;
		const TCHAR* TargetPackage;
	};

	const TArray<TPair<FString, FString>>& GetLegacyBumperIdPairs()
	{
		static const TArray<TPair<FString, FString>> Pairs =
		{
			{TEXT("Rebound_CounterShell"), TEXT("Rebound_KineticShell")},
			{TEXT("Rebound_ManaOrb"), TEXT("Rebound_BloodOverdrive")},
			{TEXT("Side_LaunchCharge"), TEXT("Side_CounterShield")},
			{TEXT("Top_ComboUp"), TEXT("Top_ComboArc")},
			{TEXT("Top_ComboPickup"), TEXT("Top_VulnerabilityShell")},
			{TEXT("Gate_ManaField"), TEXT("Gate_ReactiveRepair")},
			{TEXT("Gate_StrengthField"), TEXT("Gate_ManaReactor")}
		};
		return Pairs;
	}

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

	FName ReplaceLegacyBumperTokens(const FName Value)
	{
		if (Value.IsNone())
		{
			return NAME_None;
		}

		FString Result = Value.ToString();
		for (const TPair<FString, FString>& Pair : GetLegacyBumperIdPairs())
		{
			Result.ReplaceInline(*Pair.Key, *Pair.Value, ESearchCase::CaseSensitive);
		}
		return FName(*Result);
	}

	bool DuplicateCounterShieldBlueprint(IAssetTools& AssetTools)
	{
		const FString LegacySourcePackage = TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_01");
		const FString CurrentSourcePackage = TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_02");
		const FString TargetPackage = TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_CounterShield_01");
		if (LoadAsset(TargetPackage))
		{
			return true;
		}

		UObject* SourceAsset = LoadAsset(LegacySourcePackage);
		if (!IsValid(SourceAsset))
		{
			SourceAsset = LoadAsset(CurrentSourcePackage);
		}
		if (!IsValid(SourceAsset))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] CounterShield source is missing. Legacy=%s Current=%s"),
				*LegacySourcePackage,
				*CurrentSourcePackage);
			return false;
		}

		UObject* DuplicatedAsset = AssetTools.DuplicateAsset(
			TEXT("BP_Effect_CounterShield_01"),
			TEXT("/Game/Blueprints/Bumper/Effect"),
			SourceAsset);
		if (!IsValid(DuplicatedAsset))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] Failed to duplicate CounterShield effect Blueprint."));
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("[BumperSync] Created %s"), *TargetPackage);
		return true;
	}

	bool RenameApprovedAssets(IAssetTools& AssetTools)
	{
		static const TArray<FApprovedAssetRename> Renames =
		{
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BossDamage_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_KineticShell_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaPickup_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BloodOverdrive_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboUp_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboArc_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboPickup_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_VulnerabilityShell_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaField_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ReactiveRepair_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_StrengthField_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaReactor_01")},

			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_GroggyHammer_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_GroggyHammer_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_Shield_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_Shield_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_PartyMana_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_PartyMana_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RepairPickup_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RepairPickup_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_StrengthCharge_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_StrengthCharge_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BossGroggy_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BossGroggy_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_DirectStrike_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_DirectStrike_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboCashout_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboCashout_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RecoveryField_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RecoveryField_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_02")},

			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_CounterShell"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_KineticShell")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_ManaOrb"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_BloodOverdrive")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Side_Trigger_LaunchCharge"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Side_Trigger_CounterShield")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_ComboUp"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_ComboArc")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_ComboPickup"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_VulnerabilityShell")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_ManaField"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_ReactiveRepair")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_StrengthField"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_ManaReactor")},

			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_CounterShell"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_KineticShell")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_ManaOrb"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_BloodOverdrive")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Side_LaunchCharge"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Side_CounterShield")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_ComboUp"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_ComboArc")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_ComboPickup"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_VulnerabilityShell")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_ManaField"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_ReactiveRepair")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_StrengthField"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_ManaReactor")},

			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Rebound_CounterShell"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Rebound_KineticShell")},
			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Rebound_ManaOrb"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Rebound_BloodOverdrive")},
			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Side_LaunchCharge"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Side_CounterShield")},
			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Top_ComboUp"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Top_ComboArc")},
			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Top_ComboPickup"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Top_VulnerabilityShell")},
			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Gate_ManaField"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Gate_ReactiveRepair")},
			{TEXT("/Game/Resources/Bumper/Texture/Icon/T_Gate_StrengthField"), TEXT("/Game/Resources/Bumper/Texture/Icon/T_Gate_ManaReactor")}
		};

		TArray<FAssetRenameData> RenameData;
		for (const FApprovedAssetRename& Rename : Renames)
		{
			if (LoadAsset(Rename.TargetPackage))
			{
				continue;
			}

			UObject* SourceAsset = LoadAsset(Rename.SourcePackage);
			if (!IsValid(SourceAsset))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Rename source is missing. Source=%s Target=%s"),
					Rename.SourcePackage,
					Rename.TargetPackage);
				return false;
			}

			RenameData.Emplace(
				SourceAsset,
				FPackageName::GetLongPackagePath(Rename.TargetPackage),
				FPackageName::GetLongPackageAssetName(Rename.TargetPackage));
		}

		if (RenameData.IsEmpty())
		{
			return true;
		}

		if (!AssetTools.RenameAssets(RenameData))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] AssetTools failed to rename approved assets."));
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("[BumperSync] Renamed %d approved assets."), RenameData.Num());
		return true;
	}

	bool SetProjectileClass(UBlueprint* Blueprint)
	{
		UBlueprint* ProjectileBlueprint = Cast<UBlueprint>(
			LoadAsset(TEXT("/Game/Blueprints/Bumper/Effect/BP_TestBullet")));
		if (!IsValid(Blueprint)
			|| !IsValid(Blueprint->GeneratedClass)
			|| !IsValid(ProjectileBlueprint)
			|| !IsValid(ProjectileBlueprint->GeneratedClass))
		{
			return false;
		}

		UObject* ClassDefaultObject = Blueprint->GeneratedClass->GetDefaultObject();
		FClassProperty* ProjectileProperty = FindFProperty<FClassProperty>(
			Blueprint->GeneratedClass,
			TEXT("ProjectileClass"));
		if (!IsValid(ClassDefaultObject) || !ProjectileProperty)
		{
			return false;
		}

		ClassDefaultObject->Modify();
		ProjectileProperty->SetPropertyValue_InContainer(
			ClassDefaultObject,
			ProjectileBlueprint->GeneratedClass);
		Blueprint->MarkPackageDirty();
		return true;
	}

	bool ReparentEffectBlueprint(
		const TCHAR* PackagePath,
		UClass* NewParentClass,
		const bool bNeedsProjectileClass)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(LoadAsset(PackagePath));
		if (!IsValid(Blueprint) || !IsValid(NewParentClass))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] Cannot reparent Blueprint. Asset=%s Parent=%s"),
				PackagePath,
				*GetNameSafe(NewParentClass));
			return false;
		}

		if (Blueprint->ParentClass != NewParentClass)
		{
			UBlueprintEditorLibrary::ReparentBlueprint(Blueprint, NewParentClass);
			UBlueprintEditorLibrary::CompileBlueprint(Blueprint);
		}

		if (bNeedsProjectileClass && !SetProjectileClass(Blueprint))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] ProjectileClass could not be assigned. Asset=%s"),
				PackagePath);
			return false;
		}

		Blueprint->MarkPackageDirty();
		return Blueprint->Status != BS_Error;
	}

	bool RepairComboPulseEffectBlueprint()
	{
		return ReparentEffectBlueprint(
			TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboPulse_01"),
			UPBComboBumperEffect::StaticClass(),
			false);
	}

	bool ReparentApprovedEffectBlueprints()
	{
		return RepairComboPulseEffectBlueprint()
			&& ReparentEffectBlueprint(
			TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_KineticShell_01"),
			UPBKineticShellBumperEffect::StaticClass(),
			true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BloodOverdrive_01"),
				UPBBloodOverdriveBumperEffect::StaticClass(),
				false)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_CounterShield_01"),
				UPBCounterShieldBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboArc_01"),
				UPBComboArcBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_VulnerabilityShell_01"),
				UPBBossVulnerabilityBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_DirectStrike_02"),
				UPBBossDamageBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ReactiveRepair_01"),
				UPBGateSupportFieldBumperEffect::StaticClass(),
				false)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaReactor_01"),
				UPBGateSupportFieldBumperEffect::StaticClass(),
				false);
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
				TEXT("[BumperSync] Sheet fetch failed. Config=%s Status=%d Message=%s"),
				*GetNameSafe(Config),
				static_cast<int32>(Config->FetchStatus),
				*Config->LastMessage);
			return false;
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Sheet fetch succeeded. Config=%s Range=%s"),
			*GetNameSafe(Config),
			*Config->GetRangeString());
		return true;
	}

	UGoogleSheetConfig* ConfigureExistingSheetConfig(
		const TCHAR* PackagePath,
		const TCHAR* RangeTo)
	{
		UGoogleSheetConfig* Config = Cast<UGoogleSheetConfig>(LoadAsset(PackagePath));
		if (!IsValid(Config) || !IsValid(Config->DataParser))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] Existing Loader is invalid: %s"), PackagePath);
			return nullptr;
		}

		Config->Modify();
		Config->RangeFrom = TEXT("A1");
		Config->RangeTo = RangeTo;
		Config->bAutoSaveOnComplete = false;
		Config->MarkPackageDirty();
		return Config;
	}

	bool RefreshCollectionBumperMetadata()
	{
		UDataTable* CollectionTable = Cast<UDataTable>(
			LoadAsset(DataPath::CollectionTable));
		const UDataTable* BumperTable = Cast<UDataTable>(
			LoadAsset(DataPath::BumperTable));
		if (!IsValid(CollectionTable) || !IsValid(BumperTable))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] DT_Collection or DT_Bumper is unavailable."));
			return false;
		}
		if (CollectionTable->GetRowStruct() != FPBCollectionTableRow::StaticStruct())
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] DT_Collection has an unexpected row struct."));
			return false;
		}

		struct FRowReplacement
		{
			FName OldKey;
			FName NewKey;
			FPBCollectionTableRow Row;
		};

		TArray<FRowReplacement> Replacements;
		for (const FName RowKey : CollectionTable->GetRowNames())
		{
			const FPBCollectionTableRow* ExistingRow = CollectionTable->FindRow<FPBCollectionTableRow>(
				RowKey,
				TEXT("BumperSyncCollection"),
				false);
			if (!ExistingRow || ExistingRow->Category != EPBCollectionCategory::Bumper)
			{
				continue;
			}

			FRowReplacement& Replacement = Replacements.AddDefaulted_GetRef();
			Replacement.OldKey = RowKey;
			Replacement.NewKey = ReplaceLegacyBumperTokens(RowKey);
			Replacement.Row = *ExistingRow;
			Replacement.Row.SourceRowName = PBBumperAssetIds::NormalizeBumperRowId(
				Replacement.Row.SourceRowName);
			Replacement.Row.SourceId = Replacement.Row.SourceRowName;
			Replacement.Row.SourceTableName = TEXT("Bumper");
			Replacement.Row.CollectionId = ReplaceLegacyBumperTokens(Replacement.Row.CollectionId);
			Replacement.Row.IconAssetKey = FName(*FString::Printf(
				TEXT("T_%s"),
				*Replacement.Row.SourceRowName.ToString()));

			const FPBBumperTableRow* BumperRow = BumperTable->FindRow<FPBBumperTableRow>(
				Replacement.Row.SourceRowName,
				TEXT("BumperSyncCollection"),
				false);
			if (!BumperRow)
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Collection row points to a missing Bumper. Collection=%s Bumper=%s"),
					*RowKey.ToString(),
					*Replacement.Row.SourceRowName.ToString());
				return false;
			}

			Replacement.Row.DisplayName = BumperRow->DisplayName;
			Replacement.Row.LockedName = BumperRow->DisplayName;
			Replacement.Row.ShortDescription = BumperRow->Description;
			Replacement.Row.DetailDescription = BumperRow->Description;
			Replacement.Row.UnlockConditionText = FText::GetEmpty();
			Replacement.Row.bHiddenUntilDiscovered = false;
		}

		if (Replacements.IsEmpty())
		{
			return true;
		}

		CollectionTable->Modify();
		for (const FRowReplacement& Replacement : Replacements)
		{
			if (Replacement.NewKey != Replacement.OldKey)
			{
				CollectionTable->RemoveRow(Replacement.OldKey);
			}
			CollectionTable->AddRow(Replacement.NewKey, Replacement.Row);
		}
		CollectionTable->MarkPackageDirty();
		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Refreshed %d Bumper metadata rows in DT_Collection."),
			Replacements.Num());
		return true;
	}

	bool IsApprovedDirtyPackage(
		const FString& PackageName,
		const bool bIncludeLegacyMigrationAssets)
	{
		static const TSet<FString> ExactPackages =
		{
			DataPath::SharedEffectTable,
			DataPath::SharedEffectParamTable,
			DataPath::BumperTriggerTable,
			DataPath::BumperEffectTable,
			DataPath::BumperTable,
			DataPath::CollectionTable,
			DataPath::SharedEffectLoader,
			DataPath::SharedEffectParamLoader,
			DataPath::BumperTriggerLoader,
			DataPath::BumperEffectLoader,
			DataPath::BumperLoader
		};
		if (ExactPackages.Contains(PackageName)
			|| PackageName.StartsWith(TEXT("/Game/Data/DataAssets/Bumper/")))
		{
			return true;
		}

		return bIncludeLegacyMigrationAssets
			&& (PackageName.StartsWith(TEXT("/Game/Blueprints/Bumper/Effect/"))
				|| PackageName.StartsWith(TEXT("/Game/Blueprints/Bumper/Trigger/"))
				|| PackageName.StartsWith(TEXT("/Game/Resources/Bumper/Texture/Icon/")));
	}

	bool SaveApprovedDirtyPackages(const bool bIncludeLegacyMigrationAssets)
	{
		TArray<UPackage*> PackagesToSave;
		for (TObjectIterator<UPackage> It; It; ++It)
		{
			UPackage* Package = *It;
			if (!IsValid(Package) || !Package->IsDirty())
			{
				continue;
			}

			const FString PackageName = Package->GetName();
			if (IsApprovedDirtyPackage(PackageName, bIncludeLegacyMigrationAssets))
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
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Package has no top-level asset to save: %s"),
					*PackageName);
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
				UE_LOG(LogTemp, Error, TEXT("[BumperSync] Failed to save package: %s"), *PackageName);
				return false;
			}
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Saved %d approved dirty packages."),
			PackagesToSave.Num());
		return true;
	}

	template <typename RowType>
	const UDataTable* LoadTypedTable(const TCHAR* PackagePath, const TCHAR* Label)
	{
		const UDataTable* Table = Cast<UDataTable>(LoadAsset(PackagePath));
		if (!IsValid(Table))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] %s table is unavailable: %s"), Label, PackagePath);
			return nullptr;
		}
		if (Table->GetRowStruct() != RowType::StaticStruct())
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] %s table has an unexpected row struct: %s"), Label, PackagePath);
			return nullptr;
		}
		return Table;
	}

	bool ValidateExistingBumperData()
	{
		const UDataTable* BumperTable = LoadTypedTable<FPBBumperTableRow>(DataPath::BumperTable, TEXT("Bumper"));
		const UDataTable* TriggerTable = LoadTypedTable<FPBBumperTriggerRow>(DataPath::BumperTriggerTable, TEXT("BumperTrigger"));
		const UDataTable* EffectTable = LoadTypedTable<FPBBumperEffectRow>(DataPath::BumperEffectTable, TEXT("BumperEffect"));
		const UDataTable* SharedEffectTable = LoadTypedTable<FPBEffectTableRow>(DataPath::SharedEffectTable, TEXT("Effect"));
		const UDataTable* SharedEffectParamTable = LoadTypedTable<FPBEffectParamRow>(DataPath::SharedEffectParamTable, TEXT("EffectParam"));
		if (!BumperTable || !TriggerTable || !EffectTable || !SharedEffectTable || !SharedEffectParamTable)
		{
			return false;
		}

		const UGoogleSheetConfig* BumperLoader = Cast<UGoogleSheetConfig>(LoadAsset(DataPath::BumperLoader));
		const UGoogleSheetConfig* TriggerLoader = Cast<UGoogleSheetConfig>(LoadAsset(DataPath::BumperTriggerLoader));
		const UGoogleSheetConfig* EffectLoader = Cast<UGoogleSheetConfig>(LoadAsset(DataPath::BumperEffectLoader));
		const UGoogleSheetConfig* SharedEffectLoader = Cast<UGoogleSheetConfig>(LoadAsset(DataPath::SharedEffectLoader));
		const UGoogleSheetConfig* SharedEffectParamLoader = Cast<UGoogleSheetConfig>(LoadAsset(DataPath::SharedEffectParamLoader));
		if (!IsValid(BumperLoader) || !IsValid(BumperLoader->DataParser)
			|| !IsValid(TriggerLoader) || !IsValid(TriggerLoader->DataParser)
			|| !IsValid(EffectLoader) || !IsValid(EffectLoader->DataParser)
			|| !IsValid(SharedEffectLoader) || !IsValid(SharedEffectLoader->DataParser)
			|| !IsValid(SharedEffectParamLoader) || !IsValid(SharedEffectParamLoader->DataParser))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] One or more Bumper Loader assets are missing or invalid."));
			return false;
		}

		bool bValid = true;
		TSet<FName> ReferencedSharedEffectIds;
		for (const FName BumperRowId : BumperTable->GetRowNames())
		{
			const FPBBumperTableRow* BumperRow = BumperTable->FindRow<FPBBumperTableRow>(
				BumperRowId,
				TEXT("BumperSyncValidate"),
				false);
			if (!BumperRow)
			{
				bValid = false;
				continue;
			}

			if (BumperRow->TriggerID.IsNone()
				|| !TriggerTable->FindRow<FPBBumperTriggerRow>(
					BumperRow->TriggerID,
					TEXT("BumperSyncValidate"),
					false))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Bumper row '%s' references a missing Trigger '%s'."),
					*BumperRowId.ToString(),
					*BumperRow->TriggerID.ToString());
				bValid = false;
			}

			if (BumperRow->EffectID.IsNone()
				|| !EffectTable->FindRow<FPBBumperEffectRow>(BumperRow->EffectID, TEXT("BumperSyncValidate"), false))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Bumper row '%s' references a missing Effect '%s'."),
					*BumperRowId.ToString(),
					*BumperRow->EffectID.ToString());
				bValid = false;
			}
		}

		for (const FName BumperEffectRowId : EffectTable->GetRowNames())
		{
			const FPBBumperEffectRow* BumperEffectRow = EffectTable->FindRow<FPBBumperEffectRow>(
				BumperEffectRowId,
				TEXT("BumperSyncValidate"),
				false);
			if (!BumperEffectRow || BumperEffectRow->SharedEffectId.IsNone())
			{
				continue;
			}

			ReferencedSharedEffectIds.Add(BumperEffectRow->SharedEffectId);
			const FPBEffectTableRow* SharedEffectRow = SharedEffectTable->FindRow<FPBEffectTableRow>(
				BumperEffectRow->SharedEffectId,
				TEXT("BumperSyncValidate"),
				false);
			FString ContractError;
			if (!SharedEffectRow || !PBBumperSharedEffectAdapter::ValidateContract(
				*SharedEffectRow,
				NAME_None,
				NAME_None,
				NAME_None,
				ContractError))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Bumper Effect '%s' has an invalid shared Effect '%s': %s"),
					*BumperEffectRowId.ToString(),
					*BumperEffectRow->SharedEffectId.ToString(),
					SharedEffectRow ? *ContractError : TEXT("row is missing"));
				bValid = false;
				continue;
			}

			TSet<FName> ParamKeys;
			for (const TPair<FName, uint8*>& ParamPair : SharedEffectParamTable->GetRowMap())
			{
				const FPBEffectParamRow* ParamRow = reinterpret_cast<const FPBEffectParamRow*>(ParamPair.Value);
				if (!ParamRow || ParamRow->EffectId != BumperEffectRow->SharedEffectId)
				{
					continue;
				}
				if (ParamRow->ParamKey.IsNone() || ParamKeys.Contains(ParamRow->ParamKey))
				{
					UE_LOG(LogTemp, Error,
						TEXT("[BumperSync] Shared Effect '%s' has an empty or duplicate ParamKey '%s'."),
						*BumperEffectRow->SharedEffectId.ToString(),
						*ParamRow->ParamKey.ToString());
					bValid = false;
				}
				ParamKeys.Add(ParamRow->ParamKey);
			}
			if (ParamKeys.IsEmpty())
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Shared Effect '%s' has no parameters."),
					*BumperEffectRow->SharedEffectId.ToString());
				bValid = false;
			}
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] ValidateOnly rows. Bumper=%d Trigger=%d Effect=%d SharedEffect=%d Result=%s"),
			BumperTable->GetRowMap().Num(),
			TriggerTable->GetRowMap().Num(),
			EffectTable->GetRowMap().Num(),
			ReferencedSharedEffectIds.Num(),
			bValid ? TEXT("Success") : TEXT("Failure"));
		return bValid;
	}
}

UPBBumperDataSyncCommandlet::UPBBumperDataSyncCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UPBBumperDataSyncCommandlet::Main(const FString& Params)
{
	if (FParse::Param(*Params, TEXT("ValidateOnly")))
	{
		return ValidateExistingBumperData() ? 0 : 1;
	}

	if (FParse::Param(*Params, TEXT("RepairComboPulseParent")))
	{
		if (!RepairComboPulseEffectBlueprint() || !SaveApprovedDirtyPackages(true))
		{
			return 1;
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Combo Pulse effect parent repair completed."));
		return 0;
	}

	const bool bMigrateLegacyAssets = FParse::Param(*Params, TEXT("MigrateLegacyAssets"));
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(
		TEXT("AssetTools")).Get();

	if (bMigrateLegacyAssets
		&& (!DuplicateCounterShieldBlueprint(AssetTools)
			|| !RenameApprovedAssets(AssetTools)
			|| !ReparentApprovedEffectBlueprints()))
	{
		return 1;
	}
	if (!bMigrateLegacyAssets)
	{
		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Legacy Blueprint rename/reparent migration skipped. Use -MigrateLegacyAssets only for the approved one-time migration."));
	}

	UGoogleSheetConfig* SharedEffectConfig = ConfigureExistingSheetConfig(
		DataPath::SharedEffectLoader,
		TEXT("G1000"));
	UGoogleSheetConfig* SharedEffectParamConfig = ConfigureExistingSheetConfig(
		DataPath::SharedEffectParamLoader,
		TEXT("D2000"));

	UGoogleSheetConfig* TriggerConfig = ConfigureExistingSheetConfig(
		DataPath::BumperTriggerLoader,
		TEXT("F1000"));
	UGoogleSheetConfig* EffectConfig = ConfigureExistingSheetConfig(
		DataPath::BumperEffectLoader,
		TEXT("O1000"));
	UGoogleSheetConfig* BumperConfig = ConfigureExistingSheetConfig(
		DataPath::BumperLoader,
		TEXT("K1000"));
	if (!IsValid(SharedEffectConfig) || !IsValid(SharedEffectParamConfig)
		|| !IsValid(TriggerConfig) || !IsValid(EffectConfig) || !IsValid(BumperConfig))
	{
		return 1;
	}

	if (!FetchAndWait(SharedEffectConfig)
		|| !FetchAndWait(SharedEffectParamConfig)
		|| !FetchAndWait(TriggerConfig)
		|| !FetchAndWait(EffectConfig)
		|| !FetchAndWait(BumperConfig)
		|| !RefreshCollectionBumperMetadata())
	{
		return 1;
	}

	if (!SaveApprovedDirtyPackages(bMigrateLegacyAssets))
	{
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[BumperSync] Approved Bumper data synchronization completed."));
	return 0;
}
