// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BallStat.generated.h"

UENUM(BlueprintType)
enum class EBallStatType : uint8
{
	EST_MANA_REGEN UMETA(DisplayName = "ManaRegen"),
	EST_ATK UMETA(DisplayName = "Attack"),
	EST_STAGGER_POWER UMETA(DisplayName = "StaggerPower"),
	EST_WEIGHT UMETA(DisplayName = "Weight"),
	EST_BOUNCINESS UMETA(DisplayName = "Bounciness"),
	EST_SIZE UMETA(DisplayName = "Size"),
	EST_MAX UMETA(Hidden)
};

UINTERFACE()
class UBallStat : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PINBALLLIKE_API IBallStat
{
	GENERATED_BODY()
public:
	virtual int32 GetStat(EBallStatType Type) const = 0;
	virtual void ApplyStat(EBallStatType Type, int32 Delta) = 0;
};
