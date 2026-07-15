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

UENUM(BlueprintType)
enum class EPBRelicTargetType : uint8
{
	None,
	Ball,
	Bumper,
	Shop,
	Boss,
	Reward
};

UENUM(BlueprintType)
enum class EPBRelicModifyType : uint8
{
	Add,
	Multiply,
	Set
};