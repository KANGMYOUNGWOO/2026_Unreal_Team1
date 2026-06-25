// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperState.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperBase.generated.h"


class APBBallBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBumperTriggerCountChangedSignature,
	int32, CurrentCount,
	int32, RequiredCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBumperStateChangedSignature,
	EPBBumperState, PreviousState,
	EPBBumperState, NewState);

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API APBBumperBase : public AActor
{
	GENERATED_BODY()

public:
	APBBumperBase();

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void ActivateBumper(APBBallBase* Ball);

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void FinishActivation();

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void ResetTriggerCount();

	UFUNCTION(BlueprintCallable, Category = "Bumper")
	void SetBumperState(EPBBumperState NewState);

	UFUNCTION(BlueprintPure, Category = "Bumper")
	bool CanActivate() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	bool CanAccumulateTrigger() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	FPBBumperRuntimeData GetBumperData() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	int32 GetCurrentTriggerCount() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	int32 GetRequiredTriggerCount() const;

	UFUNCTION(BlueprintPure, Category = "Bumper")
	EPBBumperState GetBumperState() const;

	//TODO GameMessage 형태로 수정 필요.
	UPROPERTY(BlueprintAssignable, Category = "Bumper|Event")
	FPBBumperTriggerCountChangedSignature OnBumperTriggerCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Event")
	FPBBumperStateChangedSignature OnBumperStateChanged;

protected:
	void IncreaseComboCount(AActor* OtherActor, int32 Amount = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Bumper", meta = (BlueprintProtected = "true"))
	void AddTriggerCount(APBBallBase* Ball, int32 Amount = 1);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	FPBBumperRuntimeData BumperData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper")
	int32 CurrentTriggerCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper")
	EPBBumperState CurrentState = EPBBumperState::Idle;

private:
	void NotifyTriggerCountChanged();
};
