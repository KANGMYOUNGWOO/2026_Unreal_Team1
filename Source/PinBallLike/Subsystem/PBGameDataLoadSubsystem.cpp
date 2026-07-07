// Fill out your copyright notice in the Description page of Project Settings.


#include "PBGameDataLoadSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"

void UPBGameDataLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPBTableDataSubsystem::StaticClass());

	LoadStartupGameDataAsync();
}

void UPBGameDataLoadSubsystem::Deinitialize()
{
	UnloadPrimaryAssets();
	UnloadStartupGameData();

	Super::Deinitialize();
}

void UPBGameDataLoadSubsystem::LoadStartupGameDataAsync()
{
	UnloadStartupGameData();

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (!IsValid(Settings))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] Missing PBGameDataSettings."));
		return;
	}

	TArray<FSoftObjectPath> TablePaths;
	TablePaths.Reserve(4);

	// 테이블 경로는 DeveloperSettings에서만 관리한다.
	const FSoftObjectPath CollectionTablePath = Settings->CollectionTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTablePath = Settings->BumperTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTriggerTablePath = Settings->BumperTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath BumperEffectTablePath = Settings->BumperEffectTable.ToSoftObjectPath();

	if (CollectionTablePath.IsValid())
	{
		TablePaths.Add(CollectionTablePath);
	}

	if (BumperTablePath.IsValid())
	{
		TablePaths.Add(BumperTablePath);
	}

	if (BumperTriggerTablePath.IsValid())
	{
		TablePaths.Add(BumperTriggerTablePath);
	}

	if (BumperEffectTablePath.IsValid())
	{
		TablePaths.Add(BumperEffectTablePath);
	}

	if (TablePaths.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No startup table paths are configured."));
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	StartupGameDataLoadHandle = StreamableManager.RequestAsyncLoad(
		TablePaths,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBGameDataLoadSubsystem::OnStartupGameDataLoadedInternal,
			TablePaths));
}

void UPBGameDataLoadSubsystem::LoadPrimaryAssetsAsync(
	const TArray<FPrimaryAssetType>& AssetTypes,
	const TArray<FName>& BundleNames)
{
	UnloadPrimaryAssets();

	TArray<FPrimaryAssetId> AssetIds;
	for (const FPrimaryAssetType& AssetType : AssetTypes)
	{
		AppendPrimaryAssetIds(AssetType, AssetIds);
	}

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary assets found. TypeCount=%d"),
			AssetTypes.Num());
		CompletePrimaryAssetLoad(false);
		return;
	}

	LoadPrimaryAssetsByIdsAsync(
		AssetIds,
		BundleNames,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBGameDataLoadSubsystem::OnPrimaryAssetsLoadedInternal,
			AssetIds));
}

void UPBGameDataLoadSubsystem::LoadPrimaryAssetTypeAsync(
	const FPrimaryAssetType AssetType,
	const TArray<FName>& BundleNames)
{
	TArray<FPrimaryAssetType> AssetTypes;
	AssetTypes.Add(AssetType);
	LoadPrimaryAssetsAsync(AssetTypes, BundleNames);
}

void UPBGameDataLoadSubsystem::LoadPrimaryAssetsByIdsAsync(
	const TArray<FPrimaryAssetId>& AssetIds,
	const TArray<FName>& BundleNames,
	FStreamableDelegate OnLoaded)
{
	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary asset ids."));
		CompletePrimaryAssetLoad(false);
		return;
	}

	const FName BundleKey = MakePrimaryAssetBundleKey(BundleNames);
	UnloadPrimaryAssetBundle(BundleKey);
	bIsPrimaryAssetsReady = false;

	PrimaryAssetLoadHandlesByBundle.Add(BundleKey, UAssetManager::Get().LoadPrimaryAssets(
		AssetIds,
		BundleNames,
		OnLoaded));

	LoadedPrimaryAssetIdsByBundle.Add(BundleKey, AssetIds);
}

void UPBGameDataLoadSubsystem::LoadSoftReferencesAsync(
	const TArray<FSoftObjectPath>& SoftReferencePaths,
	FStreamableDelegate OnLoaded)
{
	if (SoftReferenceLoadHandle.IsValid())
	{
		SoftReferenceLoadHandle->ReleaseHandle();
		SoftReferenceLoadHandle.Reset();
	}

	if (SoftReferencePaths.IsEmpty())
	{
		OnLoaded.ExecuteIfBound();
		return;
	}

	SoftReferenceLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SoftReferencePaths,
		OnLoaded);
}

void UPBGameDataLoadSubsystem::LoadPrimaryAssetsByNamesAsync(
	const FPrimaryAssetType AssetType,
	const TArray<FName>& AssetNames,
	const TArray<FName>& BundleNames)
{
	PendingPrimaryAssetType = AssetType;
	PendingPrimaryAssetNames = AssetNames;
	PendingPrimaryAssetBundleNames = BundleNames;

	if (!IsStartupGameDataReady())
	{
		OnStartupGameDataLoaded.AddUniqueDynamic(
			this,
			&UPBGameDataLoadSubsystem::ResumePrimaryAssetLoadByNamesAfterStartupDataReady);
		return;
	}

	TArray<FPrimaryAssetId> AssetIds;
	AssetIds.Reserve(PendingPrimaryAssetNames.Num());
	for (const FName& AssetName : PendingPrimaryAssetNames)
	{
		if (!AssetName.IsNone())
		{
			AssetIds.Add(FPrimaryAssetId(PendingPrimaryAssetType, AssetName));
		}
	}

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary asset names. Type=%s"),
			*PendingPrimaryAssetType.ToString());
		CompletePrimaryAssetLoad(false);
		return;
	}

	LoadPrimaryAssetsByIdsAsync(
		AssetIds,
		PendingPrimaryAssetBundleNames,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBGameDataLoadSubsystem::OnPrimaryAssetsLoadedInternal,
			AssetIds));
}

void UPBGameDataLoadSubsystem::ResumePrimaryAssetLoadByNamesAfterStartupDataReady()
{
	OnStartupGameDataLoaded.RemoveDynamic(
		this,
		&UPBGameDataLoadSubsystem::ResumePrimaryAssetLoadByNamesAfterStartupDataReady);

	LoadPrimaryAssetsByNamesAsync(
		PendingPrimaryAssetType,
		PendingPrimaryAssetNames,
		PendingPrimaryAssetBundleNames);
}

void UPBGameDataLoadSubsystem::UnloadStartupGameData()
{
	LoadedStartupTables.Empty();
	bIsStartupGameDataReady = false;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBTableDataSubsystem* TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>())
		{
			TableDataSubsystem->SetCollectionTable(nullptr);
			TableDataSubsystem->SetBumperTables(nullptr, nullptr, nullptr);
		}
	}

	if (StartupGameDataLoadHandle.IsValid())
	{
		StartupGameDataLoadHandle->ReleaseHandle();
		StartupGameDataLoadHandle.Reset();
	}
}

void UPBGameDataLoadSubsystem::UnloadPrimaryAssets()
{
	LoadedPrimaryAssets.Empty();
	LoadedPrimaryAssetIdsByBundle.Empty();
	bIsPrimaryAssetsReady = false;

	// 핸들을 해제하면 Subsystem이 유지하던 스트리밍 참조가 정리된다.
	for (TPair<FName, TSharedPtr<FStreamableHandle>>& HandlePair : PrimaryAssetLoadHandlesByBundle)
	{
		if (HandlePair.Value.IsValid())
		{
			HandlePair.Value->ReleaseHandle();
			HandlePair.Value.Reset();
		}
	}
	PrimaryAssetLoadHandlesByBundle.Empty();

	if (SoftReferenceLoadHandle.IsValid())
	{
		SoftReferenceLoadHandle->ReleaseHandle();
		SoftReferenceLoadHandle.Reset();
	}
}

void UPBGameDataLoadSubsystem::UnloadPrimaryAssetBundle(const FName BundleName)
{
	TSharedPtr<FStreamableHandle> LoadHandle;
	if (PrimaryAssetLoadHandlesByBundle.RemoveAndCopyValue(BundleName, LoadHandle) && LoadHandle.IsValid())
	{
		LoadHandle->ReleaseHandle();
		LoadHandle.Reset();
	}

	RemoveLoadedPrimaryAssetsForBundle(BundleName);
	bIsPrimaryAssetsReady = LoadedPrimaryAssets.Num() > 0;
}

void UPBGameDataLoadSubsystem::UnloadPrimaryAssetBundleByType(const EPBAssetBundleType BundleType)
{
	UnloadPrimaryAssetBundle(PBAssetBundleNames::ToName(BundleType));
}

UObject* UPBGameDataLoadSubsystem::GetLoadedPrimaryAsset(const FPrimaryAssetId PrimaryAssetId) const
{
	const TObjectPtr<UObject>* LoadedAsset = LoadedPrimaryAssets.Find(PrimaryAssetId);
	return LoadedAsset ? LoadedAsset->Get() : nullptr;
}

void UPBGameDataLoadSubsystem::OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths)
{
	LoadedStartupTables.Empty();

	UDataTable* CollectionTable = nullptr;
	UDataTable* BumperTable = nullptr;
	UDataTable* BumperTriggerTable = nullptr;
	UDataTable* BumperEffectTable = nullptr;

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (IsValid(Settings))
	{
		// RequestAsyncLoad 완료 후 실제 테이블을 조회 Subsystem에 전달한다.
		CollectionTable = Cast<UDataTable>(Settings->CollectionTable.Get());
		BumperTable = Cast<UDataTable>(Settings->BumperTable.Get());
		BumperTriggerTable = Cast<UDataTable>(Settings->BumperTriggerTable.Get());
		BumperEffectTable = Cast<UDataTable>(Settings->BumperEffectTable.Get());
	}

	for (const FSoftObjectPath& LoadedPath : LoadedPaths)
	{
		if (UDataTable* LoadedTable = Cast<UDataTable>(LoadedPath.ResolveObject()))
		{
			LoadedStartupTables.Add(LoadedTable);
		}
	}

	if (UPBTableDataSubsystem* TableDataSubsystem = GetGameInstance()->GetSubsystem<UPBTableDataSubsystem>())
	{
		TableDataSubsystem->SetCollectionTable(CollectionTable);
		TableDataSubsystem->SetBumperTables(BumperTable, BumperTriggerTable, BumperEffectTable);
	}

	bIsStartupGameDataReady = IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable);

	UE_LOG(LogTemp, Log, TEXT("[GameDataLoad] Startup game data loaded. Ready=%s TableCount=%d"),
		bIsStartupGameDataReady ? TEXT("true") : TEXT("false"),
		LoadedStartupTables.Num());

	OnStartupGameDataLoaded.Broadcast();
}

void UPBGameDataLoadSubsystem::OnPrimaryAssetsLoadedInternal(TArray<FPrimaryAssetId> LoadedAssetIds)
{
	CachePrimaryAssetsFromManager(LoadedAssetIds);
	CompletePrimaryAssetLoad(LoadedPrimaryAssets.Num() > 0);
}

void UPBGameDataLoadSubsystem::CachePrimaryAssetsFromManager(const TArray<FPrimaryAssetId>& AssetIds)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (UObject* LoadedAsset = AssetManager.GetPrimaryAssetObject(AssetId))
		{
			LoadedPrimaryAssets.Add(AssetId, LoadedAsset);
		}
	}
}

void UPBGameDataLoadSubsystem::CompletePrimaryAssetLoad(const bool bReady)
{
	bIsPrimaryAssetsReady = bReady;

	UE_LOG(LogTemp, Log, TEXT("[GameDataLoad] Primary assets loaded. Ready=%s AssetCount=%d"),
		bIsPrimaryAssetsReady ? TEXT("true") : TEXT("false"),
		LoadedPrimaryAssets.Num());

	OnPrimaryAssetsLoaded.Broadcast();
}

void UPBGameDataLoadSubsystem::AppendPrimaryAssetIds(
	const FPrimaryAssetType AssetType,
	TArray<FPrimaryAssetId>& OutAssetIds) const
{
	TArray<FPrimaryAssetId> AssetIds;
	UAssetManager::Get().GetPrimaryAssetIdList(AssetType, AssetIds);
	OutAssetIds.Append(AssetIds);
}

FName UPBGameDataLoadSubsystem::MakePrimaryAssetBundleKey(const TArray<FName>& BundleNames) const
{
	if (BundleNames.IsEmpty())
	{
		return NAME_None;
	}

	if (BundleNames.Num() == 1)
	{
		return BundleNames[0];
	}

	FString BundleKey;
	for (const FName& BundleName : BundleNames)
	{
		if (!BundleKey.IsEmpty())
		{
			BundleKey.AppendChar(TEXT('+'));
		}

		BundleKey.Append(BundleName.ToString());
	}

	return FName(*BundleKey);
}

void UPBGameDataLoadSubsystem::RemoveLoadedPrimaryAssetsForBundle(const FName BundleName)
{
	TArray<FPrimaryAssetId> AssetIds;
	if (!LoadedPrimaryAssetIdsByBundle.RemoveAndCopyValue(BundleName, AssetIds))
	{
		return;
	}

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (!IsPrimaryAssetLoadedByOtherBundle(BundleName, AssetId))
		{
			LoadedPrimaryAssets.Remove(AssetId);
		}
	}
}

bool UPBGameDataLoadSubsystem::IsPrimaryAssetLoadedByOtherBundle(
	const FName BundleName,
	const FPrimaryAssetId PrimaryAssetId) const
{
	for (const TPair<FName, TArray<FPrimaryAssetId>>& BundleAssetIds : LoadedPrimaryAssetIdsByBundle)
	{
		if (BundleAssetIds.Key != BundleName && BundleAssetIds.Value.Contains(PrimaryAssetId))
		{
			return true;
		}
	}

	return false;
}
