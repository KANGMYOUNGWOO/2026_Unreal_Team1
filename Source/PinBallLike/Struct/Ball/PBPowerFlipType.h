// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBPowerFlipType.generated.h"

UENUM(BlueprintType)
enum class EPBPowerFlipType : uint8
{
	Sword = 0 UMETA(DisplayName = "Sword"),
	Fighting = 1 UMETA(DisplayName = "Fighting"),
	Shooting = 2 UMETA(DisplayName = "Shooting"),
	Special = 3 UMETA(DisplayName = "Special")
};
