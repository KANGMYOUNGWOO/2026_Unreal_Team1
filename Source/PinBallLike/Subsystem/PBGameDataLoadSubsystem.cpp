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
	TablePaths.Reserve(6);

	// 테이블 경로는 DeveloperSettings에서만 관리한다.
	const FSoftObjectPath CollectionTablePath = Settings->CollectionTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTablePath = Settings->BumperTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTriggerTablePath = Settings->BumperTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath BumperEffectTablePath = Settings->BumperEffectTable.ToSoftObjectPath();
	const FSoftObjectPath BallTablePath = Settings->BallTable.ToSoftObjectPath();
	const FSoftObjectPath BallStarLevelTablePath = Settings->BallStarLevelTable.ToSoftObjectPath();
    //const FSoftObjectPath ShopTablePath = Settings->ShopTable.ToSoftObjectPath();
	
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

FGuid UPBGameDataLoadSubsystem::LoadPrimaryAssetsAsync(
	const TArray<FPrimaryAssetType>& AssetTypes,
	const TArray<FName>& BundleNames)
{
	TArray<FPrimaryAssetId> AssetIds;
	for (const FPrimaryAssetType& AssetType : AssetTypes)
	{
		AppendPrimaryAssetIds(AssetType, AssetIds);
	}

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary assets found. TypeCount=%d"),
			AssetTypes.Num());
		bIsPrimaryAssetsReady = LoadedPrimaryAssets.Num() > 0;
		OnPrimaryAssetsLoaded.Broadcast();
		return FGuid();
	}

	return LoadPrimaryAssetsByIdsAsync(
		AssetIds,
		BundleNames,
		FStreamableDelegate());
}

FGuid UPBGameDataLoadSubsystem::LoadPrimaryAssetTypeAsync(
	const FPrimaryAssetType AssetType,
	const TArray<FName>& BundleNames)
{
	TArray<FPrimaryAssetType> AssetTypes;
	AssetTypes.Add(AssetType);
	return LoadPrimaryAssetsAsync(AssetTypes, BundleNames);
}

FGuid UPBGameDataLoadSubsystem::LoadPrimaryAssetsByIdsAsync(
	const TArray<FPrimaryAssetId>& AssetIds,
	const TArray<FName>& BundleNames,
	FStreamableDelegate OnLoaded)
{
	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary asset ids."));
		bIsPrimaryAssetsReady = LoadedPrimaryAssets.Num() > 0;
		OnLoaded.ExecuteIfBound();
		OnPrimaryAssetsLoaded.Broadcast();
		return FGuid();
	}

	return CreatePrimaryAssetLoadRequest(AssetIds, BundleNames, OnLoaded);
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

FGuid UPBGameDataLoadSubsystem::LoadPrimaryAssetsByNamesAsync(
	const FPrimaryAssetType AssetType,
	const TArray<FName>& AssetNames,
	const TArray<FName>& BundleNames)
{
	TArray<FPrimaryAssetId> AssetIds;
	AssetIds.Reserve(AssetNames.Num());
	for (const FName& AssetName : AssetNames)
	{
		if (!AssetName.IsNone())
		{
			AssetIds.Add(FPrimaryAssetId(AssetType, AssetName));
		}
	}

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] No primary asset names. Type=%s"),
			*AssetType.ToString());
		bIsPrimaryAssetsReady = LoadedPrimaryAssets.Num() > 0;
		OnPrimaryAssetsLoaded.Broadcast();
		return FGuid();
	}

	return LoadPrimaryAssetsByIdsAsync(
		AssetIds,
		BundleNames,
		FStreamableDelegate());
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
	LoadedPrimaryAssetIdsByBundle.Empty();
	ActivePrimaryAssetLoadRequests.Empty();
	bIsPrimaryAssetsReady = false;

	// 핸들을 해제하면 Subsystem이 유지하던 스트리밍 참조가 정리된다.
	for (TPair<FGuid, TSharedPtr<FStreamableHandle>>& HandlePair : ActivePrimaryAssetLoadHandles)
	{
		if (HandlePair.Value.IsValid())
		{
			HandlePair.Value->ReleaseHandle();
			HandlePair.Value.Reset();
		}
	}
	ActivePrimaryAssetLoadHandles.Empty();

	if (SoftReferenceLoadHandle.IsValid())
	{
		SoftReferenceLoadHandle->ReleaseHandle();
		SoftReferenceLoadHandle.Reset();
	}
}

void UPBGameDataLoadSubsystem::UnloadPrimaryAssetBundle(const FName BundleName)
{
	TArray<FGuid> RequestIdsToRemove;
	for (const TPair<FGuid, FPBPrimaryAssetLoadRequest>& RequestPair : ActivePrimaryAssetLoadRequests)
	{
		if (RequestPair.Value.BundleKey == BundleName)
		{
			RequestIdsToRemove.Add(RequestPair.Key);
		}
	}

	for (const FGuid& RequestId : RequestIdsToRemove)
	{
		TSharedPtr<FStreamableHandle> LoadHandle;
		if (ActivePrimaryAssetLoadHandles.RemoveAndCopyValue(RequestId, LoadHandle) && LoadHandle.IsValid())
		{
			LoadHandle->ReleaseHandle();
			LoadHandle.Reset();
		}

		ActivePrimaryAssetLoadRequests.Remove(RequestId);
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

bool UPBGameDataLoadSubsystem::IsPrimaryAssetLoadRequestActive(const FGuid RequestId) const
{
	const TSharedPtr<FStreamableHandle>* LoadHandle = ActivePrimaryAssetLoadHandles.Find(RequestId);
	return LoadHandle && LoadHandle->IsValid() && (*LoadHandle)->IsLoadingInProgress();
}

bool UPBGameDataLoadSubsystem::IsPrimaryAssetBundleLoaded(const FName BundleName) const
{
	return LoadedPrimaryAssetIdsByBundle.Contains(BundleName);
}

void UPBGameDataLoadSubsystem::OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths)
{
	LoadedStartupTables.Empty();

	UDataTable* CollectionTable = nullptr;
	UDataTable* BumperTable = nullptr;
	UDataTable* BumperTriggerTable = nullptr;
	UDataTable* BumperEffectTable = nullptr;
	UDataTable* BallTable = nullptr;
	UDataTable* BallStarLevelTable = nullptr;

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (IsValid(Settings))
	{
		// RequestAsyncLoad 완료 후 실제 테이블을 조회 Subsystem에 전달한다.
		CollectionTable = Cast<UDataTable>(Settings->CollectionTable.Get());
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
		TableDataSubsystem->SetCollectionTable(CollectionTable);
		TableDataSubsystem->SetBumperTables(BumperTable, BumperTriggerTable, BumperEffectTable);
		TableDataSubsystem->SetBallTables(BallTable, BallStarLevelTable);
	}

	bIsStartupGameDataReady = IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable)
		&& IsValid(BallTable)
		&& IsValid(BallStarLevelTable)
		&& IsValid(CollectionTable);

	UE_LOG(LogTemp, Log, TEXT("[GameDataLoad] Startup game data loaded. Ready=%s TableCount=%d"),
		bIsStartupGameDataReady ? TEXT("true") : TEXT("false"),
		LoadedStartupTables.Num());

	OnStartupGameDataLoaded.Broadcast();
}

void UPBGameDataLoadSubsystem::AppendPrimaryAssetIds(
	const FPrimaryAssetType AssetType,
	TArray<FPrimaryAssetId>& OutAssetIds) const
{
	TArray<FPrimaryAssetId> AssetIds;
	UAssetManager::Get().GetPrimaryAssetIdList(AssetType, AssetIds);
	OutAssetIds.Append(AssetIds);
}

FGuid UPBGameDataLoadSubsystem::CreatePrimaryAssetLoadRequest(
	const TArray<FPrimaryAssetId>& AssetIds,
	const TArray<FName>& BundleNames,
	FStreamableDelegate OnLoaded)
{
	FPBPrimaryAssetLoadRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.AssetIds = AssetIds;
	Request.BundleNames = BundleNames;
	Request.BundleKey = MakePrimaryAssetBundleKey(BundleNames);
	Request.OnLoaded = OnLoaded;

	StartPrimaryAssetLoadRequest(Request);
	return Request.RequestId;
}

void UPBGameDataLoadSubsystem::StartPrimaryAssetLoadRequest(const FPBPrimaryAssetLoadRequest& Request)
{
	bIsPrimaryAssetsReady = false;

	ActivePrimaryAssetLoadRequests.Add(Request.RequestId, Request);
	TSharedPtr<FStreamableHandle> LoadHandle = UAssetManager::Get().LoadPrimaryAssets(
		Request.AssetIds,
		Request.BundleNames,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBGameDataLoadSubsystem::OnPrimaryAssetsLoadedInternal,
			Request.RequestId));

	if (!LoadHandle.IsValid())
	{
		FPBPrimaryAssetLoadRequest RemovedRequest;
		ActivePrimaryAssetLoadRequests.RemoveAndCopyValue(Request.RequestId, RemovedRequest);
		CompletePrimaryAssetLoad(Request, TArray<FPrimaryAssetId>());
		return;
	}

	if (ActivePrimaryAssetLoadRequests.Contains(Request.RequestId))
	{
		ActivePrimaryAssetLoadHandles.Add(Request.RequestId, LoadHandle);
	}
}

void UPBGameDataLoadSubsystem::OnPrimaryAssetsLoadedInternal(const FGuid RequestId)
{
	const FPBPrimaryAssetLoadRequest* Request = ActivePrimaryAssetLoadRequests.Find(RequestId);
	if (!Request)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] Unknown primary asset load completed. RequestId=%s"),
			*RequestId.ToString());
		return;
	}

	TArray<FPrimaryAssetId> LoadedAssetIds;
	CachePrimaryAssetsFromManager(Request->AssetIds, LoadedAssetIds);
	CompletePrimaryAssetLoad(*Request, LoadedAssetIds);
}

void UPBGameDataLoadSubsystem::CachePrimaryAssetsFromManager(
	const TArray<FPrimaryAssetId>& AssetIds,
	TArray<FPrimaryAssetId>& OutLoadedAssetIds)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (UObject* LoadedAsset = AssetManager.GetPrimaryAssetObject(AssetId))
		{
			LoadedPrimaryAssets.Add(AssetId, LoadedAsset);
			OutLoadedAssetIds.Add(AssetId);
		}
	}
}

void UPBGameDataLoadSubsystem::CompletePrimaryAssetLoad(
	const FPBPrimaryAssetLoadRequest& Request,
	const TArray<FPrimaryAssetId>& LoadedAssetIds)
{
	if (!LoadedAssetIds.IsEmpty())
	{
		TArray<FPrimaryAssetId>& BundleAssetIds = LoadedPrimaryAssetIdsByBundle.FindOrAdd(Request.BundleKey);
		for (const FPrimaryAssetId& AssetId : LoadedAssetIds)
		{
			BundleAssetIds.AddUnique(AssetId);
		}
	}

	bIsPrimaryAssetsReady = LoadedPrimaryAssets.Num() > 0;

	FPBPrimaryAssetLoadResult Result;
	Result.RequestId = Request.RequestId;
	Result.BundleKey = Request.BundleKey;
	Result.RequestedAssetIds = Request.AssetIds;
	Result.LoadedAssetIds = LoadedAssetIds;
	Result.bSuccess = LoadedAssetIds.Num() == Request.AssetIds.Num();

	if (!Result.bSuccess)
	{
		FString MissingAssetIdText;
		for (const FPrimaryAssetId& RequestedAssetId : Request.AssetIds)
		{
			if (LoadedAssetIds.Contains(RequestedAssetId))
			{
				continue;
			}

			if (!MissingAssetIdText.IsEmpty())
			{
				MissingAssetIdText.Append(TEXT(", "));
			}
			MissingAssetIdText.Append(RequestedAssetId.ToString());
		}

		UE_LOG(LogTemp, Warning, TEXT("[GameDataLoad] Primary asset load incomplete. RequestId=%s Bundle=%s Missing=[%s]"),
			*Request.RequestId.ToString(),
			*Request.BundleKey.ToString(),
			*MissingAssetIdText);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameDataLoad] Primary assets loaded. RequestId=%s Bundle=%s Success=%s Loaded=%d Requested=%d TotalCached=%d"),
		*Request.RequestId.ToString(),
		*Request.BundleKey.ToString(),
		Result.bSuccess ? TEXT("true") : TEXT("false"),
		LoadedAssetIds.Num(),
		Request.AssetIds.Num(),
		LoadedPrimaryAssets.Num());

	Request.OnLoaded.ExecuteIfBound();
	OnPrimaryAssetLoadCompleted.Broadcast(Result);
	OnPrimaryAssetsLoaded.Broadcast();
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
