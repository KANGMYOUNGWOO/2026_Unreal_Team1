// Fill out your copyright notice in the Description page of Project Settings.


#include "PBGameDataLoadSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Subsystem/AssetLoader/PBBumperAssetLoader.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"

void UPBGameDataLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPBTableDataSubsystem::StaticClass());

	BumperAssetLoader = NewObject<UPBBumperAssetLoader>(this);
	if (IsValid(BumperAssetLoader))
	{
		BumperAssetLoader->Initialize(this);
	}

	LoadStartupGameDataAsync();
}

void UPBGameDataLoadSubsystem::Deinitialize()
{
	UnloadPrimaryAssets();
	UnloadStartupGameData();

	BumperAssetLoader = nullptr;

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
	TablePaths.Reserve(5);

	// 테이블 경로는 DeveloperSettings에서만 관리한다.
	const FSoftObjectPath BumperTablePath = Settings->BumperTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTriggerTablePath = Settings->BumperTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath BumperEffectTablePath = Settings->BumperEffectTable.ToSoftObjectPath();
	const FSoftObjectPath BallTablePath = Settings->BallTable.ToSoftObjectPath();
	const FSoftObjectPath BallStarLevelTablePath = Settings->BallStarLevelTable.ToSoftObjectPath();

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

	if (BallTablePath.IsValid())
	{
		TablePaths.Add(BallTablePath);
	}

	if (BallStarLevelTablePath.IsValid())
	{
		TablePaths.Add(BallStarLevelTablePath);
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
	UnloadPrimaryAssets();

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary asset ids."));
		CompletePrimaryAssetLoad(false);
		return;
	}

	PrimaryAssetLoadHandle = UAssetManager::Get().LoadPrimaryAssets(
		AssetIds,
		BundleNames,
		OnLoaded);
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

void UPBGameDataLoadSubsystem::LoadBumperAssetsAsync(
	const TArray<FName>& BumperRowIds,
	const TArray<FName>& BundleNames)
{
	if (!IsValid(BumperAssetLoader))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] Missing BumperAssetLoader."));
		CompletePrimaryAssetLoad(false);
		return;
	}

	BumperAssetLoader->LoadBumperAssetsAsync(BumperRowIds, BundleNames);
}

void UPBGameDataLoadSubsystem::UnloadStartupGameData()
{
	LoadedStartupTables.Empty();
	bStartupGameDataReady = false;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBTableDataSubsystem* TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>())
		{
			TableDataSubsystem->SetBumperTables(nullptr, nullptr, nullptr);
			TableDataSubsystem->SetBallTables(nullptr, nullptr);
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
	bPrimaryAssetsReady = false;

	// 핸들을 해제하면 Subsystem이 유지하던 스트리밍 참조가 정리된다.
	if (PrimaryAssetLoadHandle.IsValid())
	{
		PrimaryAssetLoadHandle->ReleaseHandle();
		PrimaryAssetLoadHandle.Reset();
	}

	if (SoftReferenceLoadHandle.IsValid())
	{
		SoftReferenceLoadHandle->ReleaseHandle();
		SoftReferenceLoadHandle.Reset();
	}
}

UObject* UPBGameDataLoadSubsystem::GetLoadedPrimaryAsset(const FPrimaryAssetId PrimaryAssetId) const
{
	const TObjectPtr<UObject>* LoadedAsset = LoadedPrimaryAssets.Find(PrimaryAssetId);
	return LoadedAsset ? LoadedAsset->Get() : nullptr;
}

void UPBGameDataLoadSubsystem::OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths)
{
	LoadedStartupTables.Empty();

	UDataTable* BumperTable = nullptr;
	UDataTable* BumperTriggerTable = nullptr;
	UDataTable* BumperEffectTable = nullptr;
	UDataTable* BallTable = nullptr;
	UDataTable* BallStarLevelTable = nullptr;

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (IsValid(Settings))
	{
		// RequestAsyncLoad 완료 후 실제 테이블을 조회 Subsystem에 전달한다.
		BumperTable = Cast<UDataTable>(Settings->BumperTable.Get());
		BumperTriggerTable = Cast<UDataTable>(Settings->BumperTriggerTable.Get());
		BumperEffectTable = Cast<UDataTable>(Settings->BumperEffectTable.Get());
		BallTable = Cast<UDataTable>(Settings->BallTable.Get());
		BallStarLevelTable = Cast<UDataTable>(Settings->BallStarLevelTable.Get());
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
		TableDataSubsystem->SetBumperTables(BumperTable, BumperTriggerTable, BumperEffectTable);
		TableDataSubsystem->SetBallTables(BallTable, BallStarLevelTable);
	}

	bStartupGameDataReady = IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable)
		&& IsValid(BallTable)
		&& IsValid(BallStarLevelTable);

	UE_LOG(LogTemp, Log, TEXT("[GameDataLoad] Startup game data loaded. Ready=%s TableCount=%d"),
		bStartupGameDataReady ? TEXT("true") : TEXT("false"),
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
	LoadedPrimaryAssets.Empty();

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
	bPrimaryAssetsReady = bReady;

	UE_LOG(LogTemp, Log, TEXT("[GameDataLoad] Primary assets loaded. Ready=%s AssetCount=%d"),
		bPrimaryAssetsReady ? TEXT("true") : TEXT("false"),
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
