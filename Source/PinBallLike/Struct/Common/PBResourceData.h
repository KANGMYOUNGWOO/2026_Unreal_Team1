// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBResourceData.generated.h"


USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBResourceData
{
	GENERATED_BODY()

	FPBResourceData() = default;
	FPBResourceData(FName InResourceName, float InCurrent, float InMax, float InRegenPerSecond)
		: ResourceName(InResourceName), Current(InCurrent), Max(InMax), RegenPerSecond(InRegenPerSecond)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float Current = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float Max = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float RegenPerSecond = 0.0f;
};
