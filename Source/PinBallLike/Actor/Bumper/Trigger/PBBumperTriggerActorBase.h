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

/**
 * 물리 범퍼 한 개의 판정과 독립 진행도를 담당한다.
 * 효과 실행 순서와 보상 적용은 OwnerBumper에 남겨 Trigger가 게임 보상 규칙을 소유하지 않게 한다.
 */
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

	/** 이 Trigger 인스턴스의 Count만 전달하므로 다른 위치의 게이지와 값이 섞이지 않는다. */
	UPROPERTY(BlueprintAssignable, Category = "Bumper|Trigger|Event")
	FPBBumperTriggerProgressChangedSignature OnTriggerProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Trigger|Event")
	FPBBumperTriggerProgressStateChangedSignature OnTriggerProgressStateChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Trigger", meta = (BlueprintProtected = "true"))
	void IncreaseTrigger(AActor* InteractionActor, const FHitResult& TriggerHit);

	/** 강한 물리 반발이 실제 적용된 뒤 호출하는 공통 카메라 피드백 함수다. */
	UFUNCTION(BlueprintCallable, Category = "Bumper|Feedback|Camera", meta = (BlueprintProtected = "true"))
	bool PlayImpactCameraShake(float ScaleMultiplier = 1.0f);

#pragma region Blueprint Events
	/** 새 이동 Actor 구현에서 사용하는 일반 이벤트다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Trigger")
	void OnMovableActorTriggered(AActor* InteractionActor, const FHitResult& TriggerHit);

	/** 기존 Ball 기반 Blueprint 그래프의 핀 호환을 위해 유지한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Trigger")
	void OnTriggerActivated(APBBallBase* Ball, const FHitResult& TriggerHit);
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	EPBBumperTriggerType TriggerType = EPBBumperTriggerType::HitCount;

	/** 자식 Trigger가 별도 설정을 하지 않아도 사용할 범퍼 공통 충돌 셰이크다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Camera")
	TSubclassOf<UCameraShakeBase> ImpactCameraShakeClass;

	/** 범퍼 종류별 체감 차이가 필요할 때 자식 Blueprint에서 조정하는 최종 강도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Camera",
		meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float ImpactCameraShakeScale = 1.0f;

	/** 동일 Trigger의 빠른 연속 Hit가 카메라를 계속 재시작하지 않게 하는 최소 간격이다. */
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

	double LastImpactCameraShakeTime = -1.0;
};
