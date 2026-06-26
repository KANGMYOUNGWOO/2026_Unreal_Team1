// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperState.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperTriggerActorBase.generated.h"

class APBBallBase;
class APBModularBumperBase;
class USceneComponent;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API APBBumperTriggerActorBase : public AActor
{
	GENERATED_BODY()

public:
	APBBumperTriggerActorBase();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	void InitializeTrigger(APBModularBumperBase* InOwnerBumper);

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	APBModularBumperBase* GetOwnerBumper() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	EPBBumperTriggerType GetTriggerType() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	EPBBumperState GetTriggerState() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	void SetTriggerState(EPBBumperState NewState);

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	bool CanIncreaseTrigger() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger")
	void FinishTrigger();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger", meta = (BlueprintProtected = "true"))
	void IncreaseTrigger(APBBallBase* Ball, const FHitResult& TriggerHit);

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Trigger")
	void OnTriggerActivated(APBBallBase* Ball, const FHitResult& TriggerHit);
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperTriggerType TriggerType = EPBBumperTriggerType::HitCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperState CurrentState = EPBBumperState::Idle;

	UPROPERTY()
	TWeakObjectPtr<APBModularBumperBase> OwnerBumper;
};
