#pragma once

#include "CoreMinimal.h"
#include "PBBumperRewardTypes.generated.h"

/** 범퍼가 Ball에 전달할 수 있는 공통 보상 종류입니다. */
UENUM(BlueprintType)
enum class EPBBumperRewardType : uint8
{
	/** HP, MP처럼 ResourceComponent가 소유한 수치를 회복합니다. */
	Resource,

	/** 충돌한 Ball의 콤보를 증가시킵니다. */
	Combo,

	/** 충돌한 Ball의 StatusEffectComponent에 상태효과를 적용합니다. */
	StatusEffect,

	/** 대상 Ball의 현재 공격력을 기준으로 일정 시간 비율 보정을 적용합니다. */
	TimedAttackPercent,

	/** 지정한 런타임 스탯에 일정 시간 비율 보정을 적용합니다. */
	TimedStatPercent,

	/** 실제 Resource 감소 뒤 제한된 횟수만큼 회복합니다. */
	PostDamageHeal
};
