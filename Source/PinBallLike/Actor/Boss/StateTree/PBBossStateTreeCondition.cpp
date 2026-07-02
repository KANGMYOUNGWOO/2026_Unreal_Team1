#include "PBBossStateTreeCondition.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PBBossStateTreeCondition)

bool FPBBossStateTreeCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(BossHandle);
	return true;
}

bool FPBBossStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const APBBossBase& Boss = Context.GetExternalData(BossHandle);
	bool IsPassed = false;

	switch (InstanceData.ConditionType)
	{
	case EPBBossStateTreeConditionType::Idle:
		IsPassed = Boss.IsIdleState();
		break;
	case EPBBossStateTreeConditionType::Pattern:
		IsPassed = Boss.IsPatternState();
		break;
	case EPBBossStateTreeConditionType::Groggy:
		IsPassed = Boss.IsGroggyState();
		break;
	case EPBBossStateTreeConditionType::Enraged:
		IsPassed = Boss.IsEnragedState();
		break;
	case EPBBossStateTreeConditionType::EnragedPhase:
		IsPassed = Boss.IsEnragedPhase();
		break;
	case EPBBossStateTreeConditionType::Dead:
		IsPassed = Boss.IsDeadState();
		break;
	case EPBBossStateTreeConditionType::FixedBoss:
		IsPassed = Boss.IsFixedBoss();
		break;
	case EPBBossStateTreeConditionType::MovableBoss:
		IsPassed = Boss.IsMovableBoss();
		break;
	default:
		IsPassed = false;
		break;
	}

	return IsPassed != InstanceData.IsInvert;
}
