#pragma once

#include "CoreMinimal.h"
#include "PBStatusEffectTypes.generated.h"

UENUM(BlueprintType)
enum class EPBStatusEffectStackType : uint8
{
	Replace,
	Add,
	HighestOnly,
	AddDuration
};

UENUM(BlueprintType)
enum class EPBStatusEffectDurationPolicy : uint8
{
	Permanent,
	Battle,
	Seconds,
	TriggerCount,
	HitCount,
	AttackCount
};

UENUM(BlueprintType)
enum class EPBStatusEffectModifyType : uint8
{
	Add,
	PercentAdd,
	Multiply,
	Override
};
