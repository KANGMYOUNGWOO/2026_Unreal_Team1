// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperState.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperTriggerActorBase.generated.h"

class APBBallBase;
class APBModularBumperBase;
class UPBBumperSoundComponent;
class UCameraShakeBase;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBumperTriggerProgressChangedSignature,
	int32, CurrentCount,
	int32, RequiredCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBumperTriggerProgressStateChangedSignature,
	EPBBumperTriggerProgressState, PreviousState,
	EPBBumperTriggerProgressState, NewState);

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API APBBumperTriggerActorBase : public AActor
{
	GENERATED_BODY()

public:
	APBBumperTriggerActorBase();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	void InitializeTrigger(
		APBModularBumperBase* InOwnerBumper,
		EPBBumperPositionId InPositionId,
		int32 InRequiredTriggerCount);

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	APBModularBumperBase* GetOwnerBumper() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	EPBBumperTriggerType GetTriggerType() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	EPBBumperState GetTriggerState() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	EPBBumperPositionId GetPositionId() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	int32 GetCurrentTriggerCount() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	int32 GetRequiredTriggerCount() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	EPBBumperTriggerProgressState GetTriggerProgressState() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	bool IsTriggerReady() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	void SetTriggerState(EPBBumperState NewState);

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	bool CanIncreaseTrigger() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	bool CanReactToMovableActor() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	void FinishTrigger();

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Trigger|Event")
	FPBBumperTriggerProgressChangedSignature OnTriggerProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Trigger|Event")
	FPBBumperTriggerProgressStateChangedSignature OnTriggerProgressStateChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger", meta = (BlueprintProtected = "true"))
	void IncreaseTrigger(AActor* InteractionActor, const FHitResult& TriggerHit);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Feedback|Camera", meta = (BlueprintProtected = "true"))
	bool PlayImpactCameraShake(float ScaleMultiplier = 1.0f);

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Trigger")
	void OnMovableActorTriggered(AActor* InteractionActor, const FHitResult& TriggerHit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Trigger")
	void OnTriggerActivated(APBBallBase* Ball, const FHitResult& TriggerHit);
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Feedback")
	TObjectPtr<UPBBumperSoundComponent> BumperSoundComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperTriggerType TriggerType = EPBBumperTriggerType::HitCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Camera")
	TSubclassOf<UCameraShakeBase> ImpactCameraShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Camera",
		meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float ImpactCameraShakeScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Camera",
		meta = (ClampMin = "0.0", Units = "s"))
	float MinimumImpactCameraShakeInterval = 0.08f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperState CurrentState = EPBBumperState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperPositionId PositionId = EPBBumperPositionId::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	int32 CurrentTriggerCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	int32 RequiredTriggerCount = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperTriggerProgressState ProgressState = EPBBumperTriggerProgressState::Charging;

	UPROPERTY()
	TWeakObjectPtr<APBModularBumperBase> OwnerBumper;

private:
	friend class APBModularBumperBase;

	bool AddTriggerProgress(int32 Amount);
	void ResetTriggerProgress();
	void SetTriggerProgressState(EPBBumperTriggerProgressState NewState);
	void NotifyTriggerProgressChanged();
	void PlayActivationSound();

	double LastImpactCameraShakeTime = -1.0;
};
