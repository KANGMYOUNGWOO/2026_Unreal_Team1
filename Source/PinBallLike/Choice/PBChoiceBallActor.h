// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBChoiceBallActor.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;


UCLASS()
class PINBALLLIKE_API APBChoiceBallActor : public AActor
{
	GENERATED_BODY()
	
public:

	APBChoiceBallActor();
	
	void ApplyRouteTransform(
	   const FVector& NewLocation,
	   const FRotator& NewRotation,
	   float DeltaTime);

	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void RotateBallVisual(
		const FVector& OldLocation,
		const FVector& NewLocation,
		float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, Category = "Choice Ball")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Choice Ball")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(VisibleAnywhere, Category = "Choice Ball|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Choice Ball|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Choice Ball")
	float BallRadius = 50.f;

	UPROPERTY(EditAnywhere, Category = "Choice Ball")
	bool bRotateVisual = true;
	
	UPROPERTY(EditAnywhere, Category = "Choice Ball", meta = (ClampMin = "0.1"))
	float RotationMultiplier = 1.0f;

	
};
