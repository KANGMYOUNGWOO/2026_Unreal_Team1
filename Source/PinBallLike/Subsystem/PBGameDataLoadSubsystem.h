// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PrimaryAssetId.h"
#include "PBGameDataLoadSubsystem.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct FPBPrimaryAssetLoadResult
{
	GENERATED_BODY()

	// 로드 요청을 구분하는 고유 ID. LoadPrimaryAssets 계열 함수의 반환값과 대응된다.
	UPROPERTY(BlueprintReadOnly, Category = "GameData|Asset")
	FGuid RequestId;

	// 요청에 사용한 번들 이름 묶음의 캐시 키.
	UPROPERTY(BlueprintReadOnly, Category = "GameData|Asset")
	FName BundleKey = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "GameData|Asset")
	TArray<FPrimaryAssetId> RequestedAssetIds;

	UPROPERTY(BlueprintReadOnly, Category = "GameData|Asset")
	TArray<FPrimaryAssetId> LoadedAssetIds;

	UPROPERTY(BlueprintReadOnly, Category = "GameData|Asset")
	bool bSuccess = false;
};

struct FPBPrimaryAssetLoadRequest
{
	FGuid RequestId;
	TArray<FPrimaryAssetId> AssetIds;
	TArray<FName> BundleNames;
	FName BundleKey = NAME_None;
	FStreamableDelegate OnLoaded;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBGameDataLoadEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBPrimaryAssetLoadCompletedEvent, const FPBPrimaryAssetLoadResult&, Result);

UCLASS()
class PINBALLLIKE_API UPBGameDataLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 게임 시작에 필요한 공통 테이블을 로드한다.
	void LoadStartupGameDataAsync();

	// 타입 기준으로 PrimaryAsset과 번들 리소스를 로드한다.
	FGuid LoadPrimaryAssetsAsync(
		const TArray<FPrimaryAssetType>& AssetTypes,
		const TArray<FName>& BundleNames);

	// 단일 타입 로드용 편의 함수.
	FGuid LoadPrimaryAssetTypeAsync(
		FPrimaryAssetType AssetType,
		const TArray<FName>& BundleNames);

	// 이미 알고 있는 PrimaryAssetId 목록을 로드한다.
	FGuid LoadPrimaryAssetsByIdsAsync(
		const TArray<FPrimaryAssetId>& AssetIds,
		const TArray<FName>& BundleNames,
		FStreamableDelegate OnLoaded);

	// AssetName 목록을 PrimaryAssetId로 변환해 로드하고, 생성된 요청 ID를 반환한다.
	FGuid LoadPrimaryAssetsByNamesAsync(
		FPrimaryAssetType AssetType,
		const TArray<FName>& AssetNames,
		const TArray<FName>& BundleNames);

	// PrimaryAsset 안의 SoftObject/SoftClass 참조를 추가 로드한다.
	void LoadSoftReferencesAsync(const TArray<FSoftObjectPath>& SoftReferencePaths, FStreamableDelegate OnLoaded);

	void UnloadStartupGameData();
	void UnloadPrimaryAssets();
	void UnloadPrimaryAssetBundle(FName BundleName);

	UFUNCTION(BlueprintCallable, Category = "GameData|Asset")
	void UnloadPrimaryAssetBundleByType(EPBAssetBundleType BundleType);

	bool IsStartupGameDataReady() const { return bIsStartupGameDataReady; }

	UE_DEPRECATED(5.0, "Use OnPrimaryAssetLoadCompleted with RequestId, or query a specific request/bundle state instead.")
	bool IsPrimaryAssetsReady() const { return bIsPrimaryAssetsReady; }

	// 요청 ID가 현재 AssetManager 로딩 중인 요청인지 확인한다.
	bool IsPrimaryAssetLoadRequestActive(FGuid RequestId) const;

	// 해당 번들 키로 로드 완료된 PrimaryAsset이 캐시에 남아 있는지 확인한다.
	bool IsPrimaryAssetBundleLoaded(FName BundleName) const;

	// 로드 후 캐싱된 PrimaryAsset 객체를 조회한다.
	UObject* GetLoadedPrimaryAsset(FPrimaryAssetId PrimaryAssetId) const;
	const TMap<FPrimaryAssetId, TObjectPtr<UObject>>& GetLoadedPrimaryAssets() const { return LoadedPrimaryAssets; }

	UPROPERTY(BlueprintAssignable, Category = "GameData")
	FPBGameDataLoadEvent OnStartupGameDataLoaded;

	UPROPERTY(BlueprintAssignable, Category = "GameData", meta = (DeprecatedProperty, DeprecationMessage = "Use OnPrimaryAssetLoadCompleted. This event does not identify which request completed."))
	FPBGameDataLoadEvent OnPrimaryAssetsLoaded;

	UPROPERTY(BlueprintAssignable, Category = "GameData")
	FPBPrimaryAssetLoadCompletedEvent OnPrimaryAssetLoadCompleted;

private:
	void OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths);
	void OnPrimaryAssetsLoadedInternal(FGuid RequestId);

	// 타입 하나에 맞는 PrimaryAssetId들을 결과 배열에 추가한다.
	void AppendPrimaryAssetIds(FPrimaryAssetType AssetType, TArray<FPrimaryAssetId>& OutAssetIds) const;

	// 요청 상태를 생성하고 즉시 AssetManager 로드를 시작한다.
	FGuid CreatePrimaryAssetLoadRequest(
		const TArray<FPrimaryAssetId>& AssetIds,
		const TArray<FName>& BundleNames,
		FStreamableDelegate OnLoaded);

	// 요청을 active map/handle map에 등록하고 AssetManager 비동기 로드를 시작한다.
	void StartPrimaryAssetLoadRequest(const FPBPrimaryAssetLoadRequest& Request);

	// AssetManager에서 로드된 객체를 조회해 캐시에 보관하고, 성공한 AssetId만 결과 배열에 담는다.
	void CachePrimaryAssetsFromManager(
		const TArray<FPrimaryAssetId>& AssetIds,
		TArray<FPrimaryAssetId>& OutLoadedAssetIds);

	// 요청별 완료 결과를 만들고 기존/신규 완료 이벤트를 브로드캐스트한다.
	void CompletePrimaryAssetLoad(
		const FPBPrimaryAssetLoadRequest& Request,
		const TArray<FPrimaryAssetId>& LoadedAssetIds);
	FName MakePrimaryAssetBundleKey(const TArray<FName>& BundleNames) const;
	void RemoveLoadedPrimaryAssetsForBundle(FName BundleName);
	bool IsPrimaryAssetLoadedByOtherBundle(FName BundleName, FPrimaryAssetId PrimaryAssetId) const;

	// 핸들을 유지해야 비동기 로드 결과가 GC/해제되지 않는다.
	TSharedPtr<FStreamableHandle> StartupGameDataLoadHandle;
	TSharedPtr<FStreamableHandle> SoftReferenceLoadHandle;
	TMap<FName, TArray<FPrimaryAssetId>> LoadedPrimaryAssetIdsByBundle;
	TMap<FGuid, TSharedPtr<FStreamableHandle>> ActivePrimaryAssetLoadHandles;
	TMap<FGuid, FPBPrimaryAssetLoadRequest> ActivePrimaryAssetLoadRequests;

	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedStartupTables;

	UPROPERTY()
	TMap<FPrimaryAssetId, TObjectPtr<UObject>> LoadedPrimaryAssets;

	bool bIsStartupGameDataReady = false;
	bool bIsPrimaryAssetsReady = false;
};
