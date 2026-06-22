// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallBase.generated.h"

class USphereComponent;
class UPinballBallMovementComponent;

UCLASS()
class PINBALLLIKE_API ABallBase : public AActor
{
	GENERATED_BODY()

public:
	ABallBase();

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void LaunchBall(FVector Impulse);

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void AddVelocity(FVector VelocityToAdd);

private:
	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPinballBallMovementComponent> MovementComponent;
};
