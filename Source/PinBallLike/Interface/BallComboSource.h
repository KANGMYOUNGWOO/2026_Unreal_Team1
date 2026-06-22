// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BallComboSource.generated.h"

UINTERFACE()
class UBallComboSource : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IBallComboSource
{
	GENERATED_BODY()

public:
	virtual int32 GetBallComboDelta() const = 0;
};
