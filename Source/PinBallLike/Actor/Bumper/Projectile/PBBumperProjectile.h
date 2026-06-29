// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/PBProjectile.h"
#include "PBBumperProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperProjectile : public AActor, public IPBProjectile
{
	GENERATED_BODY()

public:
	APBBumperProjectile();

	virtual void ActivateTurretProjectile_Implementation() override;
	virtual void DeactivateTurretProjectile_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
};
