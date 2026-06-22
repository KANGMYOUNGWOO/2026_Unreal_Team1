// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FPBFixedPoint
{
	static constexpr int32 Scale = 1000;

	static int32 ToRaw(float Value)
	{
		const float ScaledValue = Value * Scale;
		return Value >= 0.0f ? FMath::FloorToInt(ScaledValue) : FMath::CeilToInt(ScaledValue);
	}

	static int32 ToRawNonNegative(float Value)
	{
		return FMath::Max(0, ToRaw(Value));
	}

	static float ToFloat(int32 RawValue)
	{
		return static_cast<float>(RawValue) / Scale;
	}

	static int32 ClampRaw(int32 RawValue, int32 MinRawValue, int32 MaxRawValue)
	{
		return FMath::Clamp(RawValue, MinRawValue, MaxRawValue);
	}

	static bool IsNearlyZeroRaw(int32 RawValue)
	{
		return RawValue == 0;
	}
};
