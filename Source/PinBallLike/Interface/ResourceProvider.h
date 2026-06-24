// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ResourceProvider.generated.h"

UINTERFACE()
class UResourceProvider : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IResourceProvider
{
	GENERATED_BODY()

public:
	virtual bool HasResource(FName ResourceName) const = 0;
	virtual float GetResourceCurrent(FName ResourceName) const = 0;
	virtual float GetResourceMax(FName ResourceName) const = 0;
	virtual float GetResourceRatio(FName ResourceName) const = 0;
	virtual void SetResource(FName ResourceName, float Current, float Max) = 0;
	virtual void SetResourceCurrent(FName ResourceName, float Value) = 0;
	virtual void SetResourceMax(FName ResourceName, float Value, bool bFillCurrent) = 0;
	virtual void SetResourceRegenPerSecond(FName ResourceName, float Value) = 0;
	virtual void ApplyResourceDelta(FName ResourceName, float Delta) = 0;
	virtual bool CanConsumeResource(FName ResourceName, float Cost) const = 0;
	virtual bool ConsumeResource(FName ResourceName, float Cost) = 0;
};
