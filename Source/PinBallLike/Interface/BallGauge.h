// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BallGauge.generated.h"

UENUM(BlueprintType)
enum class EBallGaugeType : uint8
{
	EGT_HP UMETA(DisplayName = "HP"),
	EGT_MP UMETA(DisplayName = "MP"),
	EGT_MAX UMETA(Hidden)
};

UINTERFACE()
class UBallGauge : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IBallGauge
{
	GENERATED_BODY()

public:
	virtual bool HasGauge(EBallGaugeType Type) const = 0;
	virtual float GetGaugeCurrent(EBallGaugeType Type) const = 0;
	virtual float GetGaugeMax(EBallGaugeType Type) const = 0;
	virtual float GetGaugeRatio(EBallGaugeType Type) const = 0;
	virtual void SetGauge(EBallGaugeType Type, float Current, float Max) = 0;
	virtual void SetGaugeCurrent(EBallGaugeType Type, float Value) = 0;
	virtual void SetGaugeMax(EBallGaugeType Type, float Value, bool bFillCurrent) = 0;
	virtual void SetGaugeRegenPerSecond(EBallGaugeType Type, float Value) = 0;
	virtual void ApplyGaugeDelta(EBallGaugeType Type, float Delta) = 0;
	virtual bool CanConsumeGauge(EBallGaugeType Type, float Cost) const = 0;
	virtual bool ConsumeGauge(EBallGaugeType Type, float Cost) = 0;
};
