// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Projectile/ProjectileBase.h"
#include "PBBumperProjectile.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	APBBumperProjectile();
};
