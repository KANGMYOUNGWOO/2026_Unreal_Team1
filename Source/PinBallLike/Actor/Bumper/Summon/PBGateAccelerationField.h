// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PBGateAccelerationField.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

/**
 * Gate 효과가 활성화하는 가속 영역이다.
 * IMovable Actor가 영역에 새로 진입할 때 현재 진행 방향을 유지한 채 속도를 비율로 증가시킨다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateAccelerationField : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBGateAccelerationField();

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

	/** Effect 시트의 Power와 Effect BP의 지속시간을 실제 영역 설정으로 전달한다. */
	void ConfigureField(float InSpeedBoostPercent, float InActiveDuration);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Gate Field")
	TObjectPtr<UBoxComponent> FieldArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Gate Field")
	TObjectPtr<UStaticMeshComponent> FieldVisual;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Gate Field")
	float SpeedBoostPercent = 25.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Gate Field")
	float ActiveDuration = 5.0f;

private:
	TMap<TWeakObjectPtr<AActor>, int32> OverlappingActorCounts;
	/** 같은 활성화 중 영역을 나갔다 들어와도 한 Actor에는 한 번만 가속합니다. */
	TSet<TWeakObjectPtr<AActor>> AcceleratedActors;
	FTimerHandle ActiveDurationTimerHandle;

	void SetFieldActive(bool bIsActive);
	bool ApplyAcceleration(AActor* InteractionActor) const;
	void HandleActiveDurationFinished();

	UFUNCTION()
	void HandleFieldBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleFieldEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
