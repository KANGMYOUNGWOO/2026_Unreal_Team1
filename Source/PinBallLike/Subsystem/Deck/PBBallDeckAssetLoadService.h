// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/Object.h"
#include "PBBallDeckAssetLoadService.generated.h"

class APBBallBase;
class UPBBallDataAsset;
class UPBBallDeckSubsystem;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBBallDeckAssetLoadService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UPBBallDeckSubsystem* InDeckSubsystem);

	FGuid LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate OnLoaded);
	FGuid LoadPlacedBallUIAssetsAsync(FStreamableDelegate OnLoaded);
	FGuid LoadOwnedBallIconAsync(FName BallId, FStreamableDelegate OnLoaded);

	void UnloadGameplayAssets();
	void UnloadUIAssets();
	void UnloadOwnedBallIcon(FName BallId);

	const UPBBallDataAsset* GetLoadedBallDataAsset(int32 BallInstanceId) const;
	UClass* GetLoadedBallActorClass(int32 BallInstanceId) const;
	UTexture2D* GetLoadedBallIcon(int32 BallInstanceId) const;

private:
	FGuid LoadPlacedBallAssetsAsync(const TArray<FName>& BundleNames, FStreamableDelegate OnLoaded);
	void UnloadPlacedBallAssets(const TArray<FName>& BundleNames);
	bool BuildPlacedBallAssetIds(TArray<FPrimaryAssetId>& OutAssetIds) const;
	FName MakeBundleKey(const TArray<FName>& BundleNames) const;
	FName MakeOwnedBallIconBundleKey(FName BallId) const;
	bool ResolveBallDataAssetId(FName BallId, FPrimaryAssetId& OutAssetId) const;
	const UPBBallDataAsset* GetLoadedBallDataAssetById(FName BallId) const;
	bool FindBallId(int32 BallInstanceId, FName& OutBallId) const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;
};
