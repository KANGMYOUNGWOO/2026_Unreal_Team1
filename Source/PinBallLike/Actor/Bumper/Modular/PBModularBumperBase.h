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

/**
 * Trigger 배치와 효과 실행을 조정하는 범퍼 모듈이다.
 * 개별 충전값은 Trigger가 소유하고, 이 클래스는 발동 순서와 효과 실행 레인만 관리한다.
 */
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

	/** 실행 중인 효과는 취소하지 않고, 대기 요청과 나머지 Trigger의 진행도만 초기화한다. */
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

	UFUNCTION(BlueprintPure, Category = "Bumper|Trigger")
	int32 GetPendingActivationCount() const;

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
	/** 일반 이동 Actor를 지원하는 기본 효과 확장 지점이다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Bumper")
	void ApplyBumperEffectToActor(AActor* InteractionActor);
	virtual void ApplyBumperEffectToActor_Implementation(AActor* InteractionActor);

	/** 기존 Ball 기반 Blueprint의 효과 함수를 깨지 않기 위한 호환 경로다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Bumper")
	void ApplyBumperEffect(APBBallBase* Ball);
	virtual void ApplyBumperEffect_Implementation(APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnBumperReady();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnTriggerCountChanged(int32 InCurrentTriggerCount, int32 InRequiredTriggerCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper")
	void OnMovableActorActivated(AActor* InteractionActor);

	/** 기존 Ball 타입 Blueprint 이벤트 핀 호환을 위해 유지한다. */
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
	void ScheduleNextPendingActivation();
	void ProcessNextPendingActivation();
	bool HasPendingActivationFor(const APBBumperTriggerActorBase* TriggerActor) const;
	void NotifyTriggerCountChanged();

	TWeakObjectPtr<APBBumperTriggerActorBase> ActiveTriggerActor;
	TArray<FPendingBumperActivation> PendingActivations;
	bool bPendingActivationScheduled = false;
};
