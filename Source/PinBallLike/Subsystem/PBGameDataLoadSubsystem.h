// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
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

	// 게임 시작 시 필요한 전역 테이블을 비동기로 로드한다.
	void LoadStartupGameDataAsync();

	// 지정한 PrimaryAssetType들의 PrimaryDataAsset과 AssetBundle 리소스를 비동기로 로드한다.
	void LoadPrimaryAssetsAsync(const TArray<FPrimaryAssetType>& AssetTypes, const TArray<FName>& BundleNames);

	// 단일 PrimaryAssetType만 로드할 때 사용하는 편의 함수.
	void LoadPrimaryAssetTypeAsync(FPrimaryAssetType AssetType, const TArray<FName>& BundleNames);

	void UnloadStartupGameData();
	void UnloadPrimaryAssets();

	bool IsStartupGameDataReady() const { return bStartupGameDataReady; }
	bool ArePrimaryAssetsReady() const { return bPrimaryAssetsReady; }

	// 비동기 로드 후 캐싱된 PrimaryAsset 객체를 조회한다.
	UObject* GetLoadedPrimaryAsset(FPrimaryAssetId PrimaryAssetId) const;

	UPROPERTY(BlueprintAssignable, Category = "GameData")
	FPBGameDataLoadEvent OnStartupGameDataLoaded;

	UPROPERTY(BlueprintAssignable, Category = "GameData")
	FPBGameDataLoadEvent OnPrimaryAssetsLoaded;

private:
	void OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths);
	void OnPrimaryAssetsLoadedInternal(TArray<FPrimaryAssetId> LoadedAssetIds);

	// AssetManager에 로드된 PrimaryAsset 객체를 ID 기준으로 캐싱한다.
	void CacheLoadedPrimaryAssets(const TArray<FPrimaryAssetId>& AssetIds);

	// 타입 하나에 속한 PrimaryAssetId들을 결과 배열에 추가한다.
	void AppendPrimaryAssetIds(FPrimaryAssetType AssetType, TArray<FPrimaryAssetId>& OutAssetIds) const;

	// 핸들을 유지해야 비동기 로드 결과가 GC/해제되지 않는다.
	TSharedPtr<FStreamableHandle> StartupGameDataLoadHandle;
	TSharedPtr<FStreamableHandle> PrimaryAssetLoadHandle;

	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedStartupTables;

	UPROPERTY()
	TMap<FPrimaryAssetId, TObjectPtr<UObject>> LoadedPrimaryAssets;

	bool bStartupGameDataReady = false;
	bool bPrimaryAssetsReady = false;
};
