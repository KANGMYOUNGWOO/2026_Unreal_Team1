// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectKey.h"
#include "PBBallHitReactionComponent.generated.h"

class UPrimitiveComponent;
class UPBBallPhysicsComponent;
class IComboable;
class IDamageable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallHitReactionComponent();
	void InitializeDependencies(
		UPBBallPhysicsComponent* InPhysicsComponent,
		IDamageable* InDamageable,
		IComboable* InComboable);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleMovementHit(const FHitResult& Hit);
	
	void ProcessBallContact(AActor* OtherActor);
	bool WasContactProcessedThisFrame(AActor* OtherActor) const;
	void MarkContactProcessed(AActor* OtherActor);

	UPROPERTY(Transient)
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;

	IDamageable* Damageable = nullptr;
	IComboable* Comboable = nullptr;

	TSet<TObjectKey<AActor>> ProcessedContactActors;
	double ProcessedContactTime = -1.0;
};
