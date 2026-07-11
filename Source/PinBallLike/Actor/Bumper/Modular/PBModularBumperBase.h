// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperRuntimeState.h"
#include "PinBallLike/Struct/Bumper/PBBumperState.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PBModularBumperBase.generated.h"

class APBBallBase;
class APBBumperPositionAnchor;
class APBBumperTriggerActorBase;
class UPBBumperEffectBase;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBModularBumperTriggerCountChangedSignature,
	int32, CurrentCount,
	int32, RequiredCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBModularBumperStateChangedSignature,
	EPBBumperState, PreviousState,
	EPBBumperState, NewState);

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API APBModularBumperBase : public AActor
{
	GENERATED_BODY()

public:
	APBModularBumperBase();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	virtual void HandleTriggerActorActivated(
		APBBumperTriggerActorBase* TriggerActor,
		APBBallBase* Ball,
		const FHitResult& TriggerHit);

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void ActivateBumper(APBBallBase* Ball);

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void FinishActivation();

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void ResetTriggerCount();

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void SetBumperState(EPBBumperState NewState);

	UFUNCTION(BlueprintPure, Category = "Bumper")
	bool CanAccumulateTrigger() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	bool CanActivate() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	int32 GetCurrentTriggerCount() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	int32 GetRequiredTriggerCount() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	EPBBumperState GetBumperState() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	void CreateBumperEffect();

	void InitializeBumper(
		const FPBBumperTableRow& InBumperData,
		const TArray<FPBBumperTriggerSpawnInfo>& InTriggerSpawnInfos,
		const FPBBumperEffectRow& InEffectData,
		TSubclassOf<UPBBumperEffectBase> InEffectClass,
		const TMap<EPBBumperPositionId, FTransform>& InAnchorTransforms);

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Event")
	FPBModularBumperTriggerCountChangedSignature OnBumperTriggerCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Event")
	FPBModularBumperStateChangedSignature OnBumperStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AddTriggerCount(APBBallBase* Ball, int32 Amount = 1);
	APBBumperTriggerActorBase* SpawnTriggerActor(
		TSubclassOf<APBBumperTriggerActorBase> TriggerClass,
		EPBBumperPositionId PositionId);
	
	void SpawnTriggerActors(
		TSubclassOf<APBBumperTriggerActorBase> TriggerClass,
		const TArray<EPBBumperPositionId>& PositionIds);
	void SpawnTriggerActorsFromInfo(const FPBBumperTriggerSpawnInfo& SpawnInfo);
	
	void ClearTriggerActors();

#pragma region Blueprint Events
	UFUNCTION(BlueprintNativeEvent, Category = "Bumper")
	void ApplyBumperEffect(APBBallBase* Ball);
	virtual void ApplyBumperEffect_Implementation(APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperReady();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnTriggerCountChanged(int32 InCurrentTriggerCount, int32 InRequiredTriggerCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperActivated(APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperFinished();
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	FPBBumperTableRow BumperData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<FPBBumperTriggerSpawnInfo> TriggerSpawnInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	TSubclassOf<UPBBumperEffectBase> EffectClass;

	/** 시트의 Effect 행에서 읽은 실행 수치와 설명. 스폰 시 주입되며 BP 기본값에 의존하지 않는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	FPBBumperEffectRow EffectData;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	TObjectPtr<UPBBumperEffectBase> BumperEffect;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<APBBumperTriggerActorBase>> SpawnedTriggerActors;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TMap<EPBBumperPositionId, FTransform> AnchorTransforms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime")
	FPBBumperRuntimeState RuntimeState;

private:
	bool FindBumperPositionTransform(EPBBumperPositionId PositionId, FTransform& OutTransform) const;
	void NotifyTriggerCountChanged();
};
