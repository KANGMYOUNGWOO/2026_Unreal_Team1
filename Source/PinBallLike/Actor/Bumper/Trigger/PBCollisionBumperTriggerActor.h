// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PBCollisionBumperTriggerActor.generated.h"

class UPrimitiveComponent;
class APBBallBase;
class UPBBumperReactionComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBCollisionBumperTriggerActor : public APBBumperTriggerActorBase
{
	GENERATED_BODY()

public:
	APBCollisionBumperTriggerActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	FName CollisionAreaTag = TEXT("BumperCollision");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	FName TriggerAreaTag = TEXT("BumperTrigger");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<UPrimitiveComponent>> CollisionAreas;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<UPrimitiveComponent>> TriggerAreas;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Reaction")
	TObjectPtr<UPBBumperReactionComponent> ReactionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Collision", meta = (ClampMin = "0.0", Units = "cm/s"))
	float BounceVelocityStrength = 1200.0f;

	/** true면 Ball Overlap 기록 대신 충돌 지점이 Trigger Area 내부인지 검사한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision")
	bool bUseHitPointTriggerAreaValidation = false;

	/** Trigger Area 경계에서 부동소수점 오차로 유효 Hit가 빠지지 않게 하는 월드 단위 여유값이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision", meta = (ClampMin = "0.0", Units = "cm"))
	float TriggerAreaHitPointTolerance = 0.5f;

private:
	TMap<TWeakObjectPtr<APBBallBase>, int32> TriggeringBallOverlapCounts;

	void RegisterCollisionAreas();
	void SetupCollisionArea(UPrimitiveComponent* CollisionArea);
	void SetupTriggerArea(UPrimitiveComponent* TriggerArea);
	bool IsBallInTriggerArea(APBBallBase* Ball) const;
	bool IsHitPointInsideTriggerArea(const FVector& HitPoint) const;
	void AddBounceVelocityToBall(APBBallBase* Ball, const FHitResult& Hit) const;

	UFUNCTION()
	void HandleComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
