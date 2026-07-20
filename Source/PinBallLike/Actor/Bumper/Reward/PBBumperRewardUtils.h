#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"

class AActor;

struct FPBBumperRewardApplyResult
{
	bool bApplied = false;
	float AppliedValue = 0.0f;
	int32 AppliedCount = 0;
};

namespace PBBumperRewardUtils
{
	FPBBumperRewardApplyResult ApplyReward(
		AActor* TargetActor,
		EPBBumperRewardType RewardType,
		FName ResourceName,
		FName StatusEffectId,
		float RequestedPower,
		FName TimedEffectSourceId = NAME_None,
		float TimedEffectDuration = 0.0f,
		FName TimedStatName = NAME_None,
		int32 TriggerCount = 0);
}
