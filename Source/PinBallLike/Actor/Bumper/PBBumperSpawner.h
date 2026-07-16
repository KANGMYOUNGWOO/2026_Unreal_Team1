// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperSpawner.generated.h"

class APBModularBumperBase;
class UPBBumperDataAsset;
class UPBBumperEffectBase;
class UPBGameDataLoadSubsystem;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;
class USceneComponent;
class UNiagaraSystem;

struct FPBPreparedBumperSpawnData
{
	// 비동기 로드가 끝난 뒤 Modular Bumper 생성에 필요한 값만 모아둔다.
	FPBBumperTableRow BumperRow;
	TArray<FPBBumperTriggerSpawnInfo> TriggerSpawnInfos;
	FPBBumperEffectRow EffectRow;
	TSubclassOf<UPBBumperEffectBase> EffectClass;
	UNiagaraSystem* ActivationVfx = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBSpawnedBumpersReadySignature,
	bool, bSuccess,
	const TArray<APBModularBumperBase*>&, SpawnedBumperActors);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperSpawner : public AActor
{
	GENERATED_BODY()

public:
	APBBumperSpawner();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spawn")
	void CollectBumperAnchors();

	FGuid LoadEquippedBumperDataAssetAsync(const FStreamableDelegate& OnLoaded);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spawn")
	void SpawnLoadedBumpers();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spawn")
	void ClearSpawnedBumpers();

	UFUNCTION(BlueprintPure, Category = "Bumper|Spawn")
	void GetSpawnedBumpers(TArray<APBModularBumperBase*>& OutBumpers) const;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Spawn")
	FPBSpawnedBumpersReadySignature OnSpawnedBumpersReady;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BuildPendingBumperAssetIds(TArray<FPrimaryAssetId>& OutAssetIds) const;

	bool BuildPreparedBumperSpawnData();
	bool TryBuildBumperSpawnData(
		const FPBEquippedBumperSlot& EquippedSlot,
		FPBPreparedBumperSpawnData& OutSpawnData) const;
	bool TryBuildTriggerSpawnInfos(
		FName BumperRowId,
		EPBBumperEquipSlot EquipSlot,
		const UPBBumperDataAsset* BumperDataAsset,
		TArray<FPBBumperTriggerSpawnInfo>& OutTriggerSpawnInfos) const;

	void PlacePreparedBumperActors();
	APBModularBumperBase* PlaceBumperActor(const FPBPreparedBumperSpawnData& SpawnData);

	// 준비 결과를 delegate와 Gameplay Message로 알린다.
	void CompleteBumperPreparation(bool bSuccess);
	bool CacheRequiredSubsystems();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Spawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Spawn", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APBModularBumperBase> BumperClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Spawn", meta = (AllowPrivateAccess = "true"))
	TMap<EPBBumperPositionId, FTransform> AnchorTransforms;

	UPROPERTY(Transient)
	TArray<FPBEquippedBumperSlot> PendingEquippedSlots;

	TArray<FPBPreparedBumperSpawnData> PreparedBumperSpawnDataList;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBModularBumperBase>> SpawnedBumpers;

	UPROPERTY(Transient)
	TObjectPtr<UPBGameDataLoadSubsystem> CachedGameDataLoadSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBPlayerDataSubsystem> CachedPlayerDataSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBTableDataSubsystem> CachedTableDataSubsystem;
};
