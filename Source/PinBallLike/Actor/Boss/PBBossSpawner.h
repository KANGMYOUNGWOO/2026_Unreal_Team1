// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Actor.h"
#include "PBBossSpawner.generated.h"

class APBBossBase;
class UPBGameDataLoadSubsystem;
class UPBBossDataAsset;
struct FPBBattleBossDeadMessage;
class USceneComponent;
struct FGameplayTag;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBossSpawner : public AActor
{
	GENERATED_BODY()

public:
	APBBossSpawner();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Spawn")
	void SpawnBossAsync();

	FGuid LoadBossDataAssetAsync(FStreamableDelegate OnLoaded);

	UFUNCTION(BlueprintCallable, Category = "Boss|Spawn")
	void SpawnLoadedBoss();

	UFUNCTION(BlueprintPure, Category = "Boss|Spawn")
	bool IsLoadedBossDataReady() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Spawn")
	void ClearSpawnedBoss();

	void SetBossRowName(FName NewBossRowName);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleBossDataLoadCompleted();
	bool RequestBossDataAsync(FStreamableDelegate OnLoaded);
	void RegisterBossDeadEvent();
	void UnregisterBossDeadEvent();
	void HandleBossDeadMessage(FGameplayTag Channel, const FPBBattleBossDeadMessage& Message);
	void UnloadBossAssets();
	bool SpawnBossWithClass(TSubclassOf<APBBossBase> BossClassToSpawn, const UPBBossDataAsset* BossDataAsset = nullptr);
	void CompleteBossPreparation(bool IsSuccess) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APBBossBase> BossClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	FName BossRowName = TEXT("Snake");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	bool IsSpawnOnBeginPlay = false;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> SpawnedBoss;

	UPROPERTY(Transient)
	TObjectPtr<UPBGameDataLoadSubsystem> CachedGameDataLoadSubsystem;

	UPROPERTY(Transient)
	FGuid PendingBossDataLoadRequestId;

	FStreamableDelegate PendingBossDataLoadedDelegate;
	FGameplayMessageListenerHandle BossDeadListenerHandle;
	bool IsBossDataLoaded = false;
	bool IsBossAssetsUnloaded = true;
};
