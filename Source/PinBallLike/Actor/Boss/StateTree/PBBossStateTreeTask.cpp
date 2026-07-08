#include "PBBossStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PBBossStateTreeTask)

FPBBossStateTreeTask::FPBBossStateTreeTask()
{
	bShouldCallTick = false;
	bShouldCopyBoundPropertiesOnTick = false;

#if WITH_EDITORONLY_DATA
	bConsideredForCompletion = false;
	bCanEditConsideredForCompletion = false;
#endif
}

bool FPBBossStateTreeTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(BossHandle);
	return true;
}

EStateTreeRunStatus FPBBossStateTreeTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	APBBossBase& Boss = Context.GetExternalData(BossHandle);
	ExecuteAction(Boss, InstanceData.EnterAction);

	return EStateTreeRunStatus::Running;
}

void FPBBossStateTreeTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	APBBossBase& Boss = Context.GetExternalData(BossHandle);
	ExecuteAction(Boss, InstanceData.ExitAction);
}

void FPBBossStateTreeTask::ExecuteAction(APBBossBase& Boss, EPBBossStateTreeTaskAction Action) const
{
	switch (Action)
	{
	case EPBBossStateTreeTaskAction::StartIdleState:
		Boss.StartIdleState();
		break;
	case EPBBossStateTreeTaskAction::StartPatternState:
		Boss.StartPatternState();
		break;
	case EPBBossStateTreeTaskAction::SetPatternState:
		Boss.SetBossState(EPBBossState::Pattern);
		break;
	case EPBBossStateTreeTaskAction::StopPatternState:
		Boss.StopPatternState();
		break;
	case EPBBossStateTreeTaskAction::StartGroggyState:
		Boss.StartGroggyState();
		break;
	case EPBBossStateTreeTaskAction::FinishGroggyState:
		Boss.FinishGroggyState();
		break;
	case EPBBossStateTreeTaskAction::StartEnragedState:
		Boss.StartEnragedState();
		break;
	case EPBBossStateTreeTaskAction::StartDeadState:
		Boss.StartDeadState();
		break;
	case EPBBossStateTreeTaskAction::None:
	default:
		break;
	}
}
