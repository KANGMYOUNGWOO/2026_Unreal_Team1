// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BallDamageSource.generated.h"

UINTERFACE()
class UBallDamageSource : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IBallDamageSource
{
	GENERATED_BODY()

public:
	virtual int32 GetBallDamage() const = 0;
};
