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
class UNiagaraSystem;

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
		AActor* InteractionActor,
		const FHitResult& TriggerHit);

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void ActivateBumper(AActor* InteractionActor);

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

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	APBBumperTriggerActorBase* GetActiveTriggerActor() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Position")
	EPBBumperPositionId GetPrimaryPositionId() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Telemetry")
	FName GetBumperRowId() const { return BumperRowId; }

	UFUNCTION(BlueprintPure, Category = "Bumper|Telemetry")
	int32 GetMeaningfulContactCount() const { return RuntimeState.MeaningfulContactCount; }

	UFUNCTION(BlueprintPure, Category = "Bumper|Telemetry")
	int32 GetActivationCount() const { return RuntimeState.ActivationCount; }

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	int32 GetPendingActivationCount() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	void CreateBumperEffect();

	void InitializeBumper(
		FName InBumperRowId,
		const FPBBumperTableRow& InBumperData,
		const TArray<FPBBumperTriggerSpawnInfo>& InTriggerSpawnInfos,
		const FPBBumperEffectRow& InEffectData,
		TSubclassOf<UPBBumperEffectBase> InEffectClass,
		UNiagaraSystem* InActivationVfx,
		UNiagaraSystem* InDeliveryVfx,
		UNiagaraSystem* InImpactVfx,
		UNiagaraSystem* InStatusVfx,
		const TMap<EPBBumperPositionId, FTransform>& InAnchorTransforms);

	UNiagaraSystem* GetDeliveryVfx() const { return DeliveryVfx; }
	UNiagaraSystem* GetImpactVfx() const { return ImpactVfx; }
	UNiagaraSystem* GetStatusVfx() const { return StatusVfx; }

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Event")
	FPBModularBumperTriggerCountChangedSignature OnBumperTriggerCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Event")
	FPBModularBumperStateChangedSignature OnBumperStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AddTriggerCount(
		APBBumperTriggerActorBase* TriggerActor,
		AActor* InteractionActor,
		int32 Amount = 1);
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
	void ApplyBumperEffectToActor(AActor* InteractionActor);
	virtual void ApplyBumperEffectToActor_Implementation(AActor* InteractionActor);

	UFUNCTION(BlueprintNativeEvent, Category = "Bumper")
	void ApplyBumperEffect(APBBallBase* Ball);
	virtual void ApplyBumperEffect_Implementation(APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperReady();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnTriggerCountChanged(int32 InCurrentTriggerCount, int32 InRequiredTriggerCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnMovableActorActivated(AActor* InteractionActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperActivated(APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperFinished();
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	FPBBumperTableRow BumperData;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper")
	FName BumperRowId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<FPBBumperTriggerSpawnInfo> TriggerSpawnInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	TSubclassOf<UPBBumperEffectBase> EffectClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	FPBBumperEffectRow EffectData;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	TObjectPtr<UPBBumperEffectBase> BumperEffect;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	TObjectPtr<UNiagaraSystem> ActivationVfx;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	TObjectPtr<UNiagaraSystem> DeliveryVfx;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	TObjectPtr<UNiagaraSystem> ImpactVfx;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	TObjectPtr<UNiagaraSystem> StatusVfx;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<APBBumperTriggerActorBase>> SpawnedTriggerActors;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TMap<EPBBumperPositionId, FTransform> AnchorTransforms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime")
	FPBBumperRuntimeState RuntimeState;

private:
	struct FPendingBumperActivation
	{
		TWeakObjectPtr<APBBumperTriggerActorBase> TriggerActor;
		TWeakObjectPtr<AActor> InteractionActor;
	};

	bool FindBumperPositionTransform(EPBBumperPositionId PositionId, FTransform& OutTransform) const;
	APBBumperTriggerActorBase* FindReadyTrigger() const;
	void RequestActivation(APBBumperTriggerActorBase* TriggerActor, AActor* InteractionActor);
	void QueueActivation(APBBumperTriggerActorBase* TriggerActor, AActor* InteractionActor);
	void StartActivation(APBBumperTriggerActorBase* TriggerActor, AActor* InteractionActor);
	void ExecuteActivation(AActor* InteractionActor);
	void SpawnActivationVfx() const;
	void ScheduleNextPendingActivation();
	void ProcessNextPendingActivation();
	bool HasPendingActivationFor(const APBBumperTriggerActorBase* TriggerActor) const;
	void NotifyTriggerCountChanged();

	TWeakObjectPtr<APBBumperTriggerActorBase> ActiveTriggerActor;
	TArray<FPendingBumperActivation> PendingActivations;
	bool bPendingActivationScheduled = false;
};
