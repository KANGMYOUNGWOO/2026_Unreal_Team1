// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PBProjectile.generated.h"

UINTERFACE(Blueprintable)
class UPBProjectile : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IPBProjectile
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Projectile")
	void ActivateTurretProjectile();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Projectile")
	void DeactivateTurretProjectile();
};
