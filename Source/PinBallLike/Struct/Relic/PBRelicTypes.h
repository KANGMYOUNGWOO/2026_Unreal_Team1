#pragma once

#include "CoreMinimal.h"
#include "PBRelicTypes.generated.h"

UENUM(BlueprintType)
enum class EPBRelicRarity : uint8
{
	Common,
	Rare,
	Legendary
};

UENUM(BlueprintType)
enum class EPBRelicEffectType : uint8
{
	None,

	BallMaxHPAdd,
	BallMaxHPMultiply,

	BallAttackAdd,
	BallAttackMultiply,

	BallManaAdd,
	BallManaMultiply,

	BallSpeedMultiply
};