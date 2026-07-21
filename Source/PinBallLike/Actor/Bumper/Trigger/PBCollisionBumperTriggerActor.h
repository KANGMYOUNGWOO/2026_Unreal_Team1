// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PBCollisionBumperTriggerActor.generated.h"

class UPrimitiveComponent;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision")
	bool bUseHitPointTriggerAreaValidation = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision", meta = (ClampMin = "0.0", Units = "cm"))
	float TriggerAreaHitPointTolerance = 0.5f;

private:
	friend class FPBBumperRepresentativeInputPathTest;

	TMap<TWeakObjectPtr<AActor>, int32> TriggeringBallOverlapCounts;

	void RegisterCollisionAreas();
	void SetupCollisionArea(UPrimitiveComponent* CollisionArea);
	void SetupTriggerArea(UPrimitiveComponent* TriggerArea);
	bool IsBallInTriggerArea(AActor* BallActor) const;
	bool IsHitPointInsideTriggerArea(const FVector& HitPoint) const;
	bool QueueBounceVelocity(AActor* MovableActor, const FHitResult& Hit);

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
