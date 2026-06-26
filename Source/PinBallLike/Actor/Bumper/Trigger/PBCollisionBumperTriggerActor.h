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

private:
	TMap<TWeakObjectPtr<APBBallBase>, int32> TriggeringBallOverlapCounts;

	void RegisterCollisionAreas();
	void SetupCollisionArea(UPrimitiveComponent* CollisionArea);
	void SetupTriggerArea(UPrimitiveComponent* TriggerArea);
	bool IsBallInTriggerArea(APBBallBase* Ball) const;
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
