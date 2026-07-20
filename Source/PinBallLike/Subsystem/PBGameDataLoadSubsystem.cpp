// Fill out your copyright notice in the Description page of Project Settings.


#include "PBGameDataLoadSubsystem.h"

#include "Engine/AssetManager.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"

void UPBGameDataLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPBGameDataLoadSubsystem::Deinitialize()
{
	UnloadPrimaryAssets();

	Super::Deinitialize();
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

void UPBGameDataLoadSubsystem::UnloadPrimaryAssets()
{
	const int32 LoadedPrimaryAssetCount = LoadedPrimaryAssets.Num();
	const int32 LoadedBundleCount = LoadedPrimaryAssetIdsByBundle.Num();
	const int32 ActiveLoadHandleCount = ActivePrimaryAssetLoadHandles.Num();

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

	UE_LOG(LogTemp, Log,
		TEXT("[GameDataLoad] All primary assets unloaded. PrimaryAssets=%d Bundles=%d ActiveHandles=%d"),
		LoadedPrimaryAssetCount,
		LoadedBundleCount,
		ActiveLoadHandleCount);
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

	if (ArePrimaryAssetsLoadedForBundle(Request.BundleKey, Request.AssetIds))
	{
		CompletePrimaryAssetLoad(Request, Request.AssetIds);
		return Request.RequestId;
	}

	StartPrimaryAssetLoadRequest(Request);
	return ActivePrimaryAssetLoadRequests.Contains(Request.RequestId)
		? Request.RequestId
		: FGuid();
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

bool UPBGameDataLoadSubsystem::ArePrimaryAssetsLoadedForBundle(
	const FName BundleKey,
	const TArray<FPrimaryAssetId>& AssetIds) const
{
	const TArray<FPrimaryAssetId>* LoadedBundleAssetIds = LoadedPrimaryAssetIdsByBundle.Find(BundleKey);
	if (!LoadedBundleAssetIds)
	{
		return false;
	}

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		const TObjectPtr<UObject>* LoadedAsset = LoadedPrimaryAssets.Find(AssetId);
		if (!LoadedBundleAssetIds->Contains(AssetId) || !LoadedAsset || !IsValid(LoadedAsset->Get()))
		{
			return false;
		}
	}

	return true;
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
