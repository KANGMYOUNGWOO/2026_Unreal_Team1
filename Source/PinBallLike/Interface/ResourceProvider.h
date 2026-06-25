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
};
