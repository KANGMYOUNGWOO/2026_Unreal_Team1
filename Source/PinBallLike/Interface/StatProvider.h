// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StatProvider.generated.h"

UINTERFACE()
class UStatProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PINBALLLIKE_API IStatProvider
{
	GENERATED_BODY()
public:
	virtual bool HasStat(FName StatName) const = 0;
	virtual int32 GetStat(FName StatName) const = 0;
};
