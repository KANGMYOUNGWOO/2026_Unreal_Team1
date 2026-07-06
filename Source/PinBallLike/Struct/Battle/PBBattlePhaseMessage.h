// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBBattlePhaseMessage.generated.h"

UENUM(BlueprintType)
enum class EPBBattlePreparationType : uint8
{
	None UMETA(DisplayName = "None"),
	Bumper UMETA(DisplayName = "Bumper"),
	Ball UMETA(DisplayName = "Ball"),
	Boss UMETA(DisplayName = "Boss")
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePreparationCompletedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	EPBBattlePreparationType PreparationType = EPBBattlePreparationType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	bool bSuccess = false;
};
