// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PrimaryAssetId.h"
#include "PBGameDataLoadSubsystem.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBGameDataLoadEvent);

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
	void LoadPrimaryAssetsAsync(const TArray<FPrimaryAssetType>& AssetTypes, const TArray<FName>& BundleNames);

	// 단일 타입 로드용 편의 함수.
	void LoadPrimaryAssetTypeAsync(FPrimaryAssetType AssetType, const TArray<FName>& BundleNames);

	// 이미 알고 있는 PrimaryAssetId 목록을 로드한다.
	void LoadPrimaryAssetsByIdsAsync(
		const TArray<FPrimaryAssetId>& AssetIds,
		const TArray<FName>& BundleNames,
		FStreamableDelegate OnLoaded);

	void LoadPrimaryAssetsByNamesAsync(
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
	bool IsPrimaryAssetsReady() const { return bIsPrimaryAssetsReady; }

	// 로드 후 캐싱된 PrimaryAsset 객체를 조회한다.
	UObject* GetLoadedPrimaryAsset(FPrimaryAssetId PrimaryAssetId) const;
	const TMap<FPrimaryAssetId, TObjectPtr<UObject>>& GetLoadedPrimaryAssets() const { return LoadedPrimaryAssets; }

	// Loader가 공통 캐시/완료 처리를 요청할 때 사용한다.
	void CachePrimaryAssetsFromManager(const TArray<FPrimaryAssetId>& AssetIds);
	void CompletePrimaryAssetLoad(bool bReady);

	UPROPERTY(BlueprintAssignable, Category = "GameData")
	FPBGameDataLoadEvent OnStartupGameDataLoaded;

	UPROPERTY(BlueprintAssignable, Category = "GameData")
	FPBGameDataLoadEvent OnPrimaryAssetsLoaded;

private:
	UFUNCTION()
	void ResumePrimaryAssetLoadByNamesAfterStartupDataReady();

	void OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths);
	void OnPrimaryAssetsLoadedInternal(TArray<FPrimaryAssetId> LoadedAssetIds);

	// 타입 하나에 맞는 PrimaryAssetId들을 결과 배열에 추가한다.
	void AppendPrimaryAssetIds(FPrimaryAssetType AssetType, TArray<FPrimaryAssetId>& OutAssetIds) const;
	FName MakePrimaryAssetBundleKey(const TArray<FName>& BundleNames) const;
	void RemoveLoadedPrimaryAssetsForBundle(FName BundleName);
	bool IsPrimaryAssetLoadedByOtherBundle(FName BundleName, FPrimaryAssetId PrimaryAssetId) const;

	// 핸들을 유지해야 비동기 로드 결과가 GC/해제되지 않는다.
	TSharedPtr<FStreamableHandle> StartupGameDataLoadHandle;
	TSharedPtr<FStreamableHandle> SoftReferenceLoadHandle;
	TMap<FName, TSharedPtr<FStreamableHandle>> PrimaryAssetLoadHandlesByBundle;
	TMap<FName, TArray<FPrimaryAssetId>> LoadedPrimaryAssetIdsByBundle;

	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedStartupTables;

	UPROPERTY()
	TMap<FPrimaryAssetId, TObjectPtr<UObject>> LoadedPrimaryAssets;

	UPROPERTY(Transient)
	FPrimaryAssetType PendingPrimaryAssetType;

	UPROPERTY(Transient)
	TArray<FName> PendingPrimaryAssetNames;

	UPROPERTY(Transient)
	TArray<FName> PendingPrimaryAssetBundleNames;

	bool bIsStartupGameDataReady = false;
	bool bIsPrimaryAssetsReady = false;
};
