// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Common/PBResourceData.h"
#include "PinBallLike/Struct/Common/PBStatData.h"
#include "PBBallStarLevelData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallStarLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TArray<FPBResourceData> BaseResources;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	TArray<FPBStatData> BaseStats;
};
