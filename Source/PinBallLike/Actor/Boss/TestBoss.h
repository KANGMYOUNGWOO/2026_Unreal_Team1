// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TestBoss.generated.h"

class ABallBase;
class UPrimitiveComponent;
class USphereComponent;



UCLASS()
class PINBALLLIKE_API ATestBoss : public APawn
{
	GENERATED_BODY()

public:
	ATestBoss();

protected:
	UFUNCTION()
	void HandleCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hittable", meta = (DisplayName = "On Hit By Ball"))
	void ReceiveHitByBall(ABallBase* Ball, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hittable", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionSphere;
};
