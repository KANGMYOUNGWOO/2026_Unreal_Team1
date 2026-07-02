// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBStatData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBStatData
{
	GENERATED_BODY()

	FPBStatData() = default;
	FPBStatData(FName InStatName, int32 InValue)
		: StatName(InStatName), Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FName StatName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	int32 Value = 0;
};