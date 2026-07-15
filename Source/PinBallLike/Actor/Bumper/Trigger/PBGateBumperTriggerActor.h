// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PBGateBumperTriggerActor.generated.h"

class UMaterialInstanceDynamic;
class UMeshComponent;
class UPrimitiveComponent;
class USkeletalMeshComponent;

/**
 * IMovable Actor가 Gate 판정 영역에 들어왔다가 완전히 빠져나오면 통과 횟수를 1 증가시킨다.
 * 여러 충돌 컴포넌트가 동시에 겹쳐도 Actor별 Overlap 수를 합산해 중복 집계를 막는다.
 * 진행도는 재질 게이지에 전달하고, Flag는 게임 판정과 독립된 Twist 회전 연출로 반응한다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateBumperTriggerActor : public APBBumperTriggerActorBase
{
	GENERATED_BODY()

public:
	APBGateBumperTriggerActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 0이면 모든 통과를 인정하고, 0보다 크면 이 속도 이상인 공만 집계한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger", meta = (ClampMin = "0.0"))
	float MinimumPassSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	FName GateAreaTag = TEXT("BumperTrigger");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<UPrimitiveComponent>> GateAreas;

	/** Count를 머티리얼 파라미터로 보여 줄 시각 Mesh의 Component Tag다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual")
	FName GaugeVisualTag = TEXT("BumperVisual");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual")
	FName GaugeParameterName = TEXT("GaugeAmount");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual", meta = (ClampMin = "0"))
	int32 GaugeMaterialIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Visual")
	float CurrentGaugeAmount = 0.0f;

	/** Gate 통과 판정과 독립적으로 Flag 세로축 회전 연출을 사용할지 결정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	bool bEnableFlagSpinReaction = true;

	/** 여러 Skeletal Mesh가 있을 때 Flag 시각 Mesh를 우선 식별할 Component Tag다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagVisualTag = TEXT("GateFlagVisual");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagBoneName = TEXT("Flag");

	/** Flag 본에서 이 끝 본을 향하는 방향을 유일한 회전축으로 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagEndBoneName = TEXT("End");

	/** Physics Asset의 Flag 관절 이름이다. 비어 있으면 FlagBoneName을 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagConstraintName = TEXT("Flag");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "cm/s"))
	float MinimumFlagSpinBallSpeed = 150.0f;

	/** 이 Ball 속도 이상에서는 MaximumFlagSpinAngularSpeed를 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "cm/s"))
	float FullFlagSpinBallSpeed = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "rad/s"))
	float MinimumFlagSpinAngularSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "rad/s"))
	float MaximumFlagSpinAngularSpeed = 34.906585f; // 2000 deg/s

	/** Gate 반폭 중 이 비율만큼 벗어난 충돌부터 최대 회전 반응을 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.01"))
	float FlagFullSpinOffsetRatio = 0.65f;

	/** 세로 회전축의 저항이다. 낮을수록 한 번 스친 뒤 더 오래 회전한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0"))
	float FlagSpinDamping = 0.45f;

	/** 아트 또는 카메라 기준과 반대로 보일 때 회전 부호만 뒤집는다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	bool bReverseFlagSpinDirection = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FVector LastFlagSpinAngularVelocity = FVector::ZeroVector;

	/** 런타임에 회전 전용 물리 초기화가 모두 성공했는지 보여 준다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	bool bIsFlagSpinReactionReady = false;

private:
	TMap<TWeakObjectPtr<AActor>, int32> PassingActorOverlapCounts;
	TMap<TWeakObjectPtr<AActor>, int32> FlagSpinActorOverlapCounts;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GaugeMaterial;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> FlagVisualMesh;

	void RegisterGateAreas();
	void SetupGateArea(UPrimitiveComponent* GateArea);
	void InitializeGaugeVisual();
	void InitializeFlagSpinReaction();
	USkeletalMeshComponent* FindFlagVisualMesh() const;
	FVector CalculateFlagSpinAxis() const;
	bool RegisterFlagSpinActorOverlap(AActor* InteractionActor);
	void UnregisterFlagSpinActorOverlap(AActor* InteractionActor);
	void ApplyFlagSpinReaction(AActor* InteractionActor, const UPrimitiveComponent* GateArea);
	bool MeetsMinimumPassSpeed(AActor* InteractionActor) const;

	UFUNCTION()
	void HandleTriggerProgressChanged(int32 CurrentCount, int32 RequiredCount);

	UFUNCTION()
	void HandleGateBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleGateEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
