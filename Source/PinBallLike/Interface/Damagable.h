// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damagable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDamagable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PINBALLLIKE_API IDamagable
{
	GENERATED_BODY()
public:
	virtual void TakeDamage(int32 Damage) = 0;
	virtual bool IsDead() const = 0;
};
