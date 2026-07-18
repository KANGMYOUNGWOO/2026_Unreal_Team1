#pragma once

#include "CoreMinimal.h"
#include "PBBumperRewardTypes.generated.h"

UENUM(BlueprintType)
enum class EPBBumperRewardType : uint8
{
	Resource,

	Combo,

	StatusEffect,

	TimedAttackPercent,

	TimedStatPercent,

	PostDamageHeal
};
