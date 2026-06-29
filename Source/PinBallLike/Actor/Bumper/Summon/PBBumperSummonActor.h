// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBumperSummonActor.generated.h"

class APBBallBase;
class APBBumperSummonActor;
class APBModularBumperBase;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBumperSummonActionFinishedSignature,
	APBBumperSummonActor*, SummonActor);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperSummonActor : public AActor
{
	GENERATED_BODY()

public:
	APBBumperSummonActor();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void FinishAction();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void DeactivateSummon();

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Summon")
	FPBBumperSummonActionFinishedSignature OnSummonActionFinished;

protected:
#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon")
	void OnStartAction(APBModularBumperBase* Bumper, APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon")
	void OnDeactivateSummon();
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Summon")
	TObjectPtr<USceneComponent> SceneRoot;
};
