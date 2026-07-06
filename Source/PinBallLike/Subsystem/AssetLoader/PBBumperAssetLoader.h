// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperAssetLoader.generated.h"

class UPBGameDataLoadSubsystem;

UCLASS()
class PINBALLLIKE_API UPBBumperAssetLoader : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UPBGameDataLoadSubsystem* InOwnerSubsystem);

	// RowId와 BundleNames를 직접 지정해 범퍼 에셋을 로드한다.
	void LoadBumperAssetsAsync(const TArray<FName>& BumperRowIds, const TArray<FName>& BundleNames);

private:
	UFUNCTION()
	void ResumeBumperAssetLoadAfterStartupDataReady();

	void HandleBumperPrimaryAssetLoadCompleted(TArray<FPrimaryAssetId> LoadedAssetIds);

	UPROPERTY()
	TObjectPtr<UPBGameDataLoadSubsystem> OwnerSubsystem;

	UPROPERTY(Transient)
	TArray<FName> PendingBumperRowIds;

	UPROPERTY(Transient)
	TArray<FName> PendingBundleNames;
};
