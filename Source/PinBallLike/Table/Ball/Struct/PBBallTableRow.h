// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Ball/PBPowerFlipType.h"
#include "PBBallTableRow.generated.h"


USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	int32 BallId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	FText DescriptionKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	EPBPowerFlipType PowerFlipType = EPBPowerFlipType::Sword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	TArray<FName> SynergyIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	TArray<FName> DefaultSkillIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	FName StarLevelId = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Table")
	FName ShopId = NAME_None;
};