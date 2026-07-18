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

/** 소환형 범퍼 효과가 재사용하는 Actor의 시작, 종료, 비활성화 계약을 정의합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperSummonActor : public AActor
{
	GENERATED_BODY()

public:
	APBBumperSummonActor();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void StartActionForActor(APBModularBumperBase* Bumper, AActor* InteractionActor);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void FinishAction();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Summon")
	virtual void DeactivateSummon();

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Summon")
	FPBBumperSummonActionFinishedSignature OnSummonActionFinished;

protected:
#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon")
	void OnStartActionForActor(APBModularBumperBase* Bumper, AActor* InteractionActor);

	/** 기존 Ball 타입 Blueprint 이벤트 핀 호환을 위해 유지한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon")
	void OnStartAction(APBModularBumperBase* Bumper, APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon")
	void OnDeactivateSummon();
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Summon")
	TObjectPtr<USceneComponent> SceneRoot;
};
