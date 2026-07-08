// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBBallStarLevelRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallStarLevelRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	FName BallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	int32 StarLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	TMap<FName, int32> BaseResources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	TMap<FName, int32> BaseStats;
};
