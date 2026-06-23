// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectKey.h"
#include "PBBallCollisionComponent.generated.h"

class UPrimitiveComponent;
class UPBBallPhysicsComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallCollisionComponent();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleMovementHit(const FHitResult& Hit);

	UFUNCTION()
	void HandleCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ProcessBallContact(AActor* OtherActor);
	bool WasContactProcessedThisFrame(AActor* OtherActor) const;
	void MarkContactProcessed(AActor* OtherActor);

	UPROPERTY(Transient)
	TObjectPtr<UPBBallPhysicsComponent> MovementComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> CollisionPrimitive;

	TSet<TObjectKey<AActor>> ProcessedContactActors;
	double ProcessedContactTime = -1.0;
};
