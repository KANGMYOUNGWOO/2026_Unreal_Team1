// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PBBumperSpawnController.generated.h"

class APBModularBumperBase;
class UPBBumperDataAsset;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;
class USceneComponent;
struct FPBBumperTableRow;
struct FPBBumperTriggerRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBSpawnedBumpersReadySignature,
	bool, bSuccess,
	const TArray<APBModularBumperBase*>&, SpawnedBumperActors);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperSpawnController : public AActor
{
	GENERATED_BODY()

public:
	APBBumperSpawnController();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spawn")
	void CollectBumperAnchors();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spawn")
	void PrepareEquippedBumpersAsync();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spawn")
	void ClearSpawnedBumpers();

	UFUNCTION(BlueprintPure, Category = "Bumper|Spawn")
	void GetSpawnedBumpers(TArray<APBModularBumperBase*>& OutBumpers) const;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Spawn")
	FPBSpawnedBumpersReadySignature OnSpawnedBumpersReady;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleEquippedBumperAssetsLoaded(const FPBPrimaryAssetLoadResult& Result);

	bool RequestEquippedBumperGameplayAssetsAsync();

	// Spawn steps.
	void SpawnPreparedBumpers();
	APBModularBumperBase* SpawnSingleBumper(FName BumperRowId);
	bool TryMakeBumperSpawnData(
		FName BumperRowId,
		FPBBumperTableRow& OutBumperRow,
		FPBBumperTriggerRow& OutTriggerRow,
		const UPBBumperDataAsset*& OutBumperDataAsset) const;
	TArray<FPBBumperTriggerSpawnInfo> MakeTriggerSpawnInfos(
		const FPBBumperTriggerRow& TriggerRow,
		const UPBBumperDataAsset* BumperDataAsset) const;
	APBModularBumperBase* SpawnInitializedBumper(
		const FPBBumperTableRow& BumperRow,
		const TArray<FPBBumperTriggerSpawnInfo>& TriggerSpawnInfos);

	// 완료 처리와 의존성 캐싱.
	void CompleteBumperPreparation(bool bSuccess);
	bool CacheRequiredSubsystems();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Spawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Spawn", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APBModularBumperBase> BumperClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Spawn", meta = (AllowPrivateAccess = "true"))
	TMap<EPBBumperPositionId, FTransform> AnchorTransforms;

	UPROPERTY(Transient)
	TArray<FName> PendingBumperRowIds;

	UPROPERTY(Transient)
	FGuid PendingBumperAssetLoadRequestId;

	UPROPERTY(Transient)
	bool bPendingBumperAssetLoadCompleted = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBModularBumperBase>> SpawnedBumpers;

	UPROPERTY(Transient)
	TObjectPtr<UPBGameDataLoadSubsystem> CachedGameDataLoadSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBPlayerDataSubsystem> CachedPlayerDataSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBTableDataSubsystem> CachedTableDataSubsystem;
};
