#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"

class AActor;

/** 범퍼 보상을 적용한 뒤 로그와 테스트에서 확인할 수 있는 실제 결과입니다. */
struct FPBBumperRewardApplyResult
{
	bool bApplied = false;
	float AppliedValue = 0.0f;
	int32 AppliedCount = 0;
};

/** 즉시 보상, 획득 아이템, 지원 영역이 같은 적용 규칙을 공유하도록 모은 범퍼 전용 유틸리티입니다. */
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
