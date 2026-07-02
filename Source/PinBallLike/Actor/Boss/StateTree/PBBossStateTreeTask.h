#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeTaskBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PBBossStateTreeTask.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeLinker;
struct FStateTreeTransitionResult;

UENUM(BlueprintType)
enum class EPBBossStateTreeTaskAction : uint8
{
	None,
	StartIdleState,
	StartPatternState,
	SetPatternState,
	StopPatternState,
	StartGroggyState,
	FinishGroggyState,
	StartEnragedState,
	StartDeadState
};

USTRUCT()
struct FPBBossStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Task")
	EPBBossStateTreeTaskAction EnterAction = EPBBossStateTreeTaskAction::None;

	UPROPERTY(EditAnywhere, Category = "Task")
	EPBBossStateTreeTaskAction ExitAction = EPBBossStateTreeTaskAction::None;
};

USTRUCT(DisplayName = "Boss State Task", meta = (Category = "Boss"))
struct FPBBossStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPBBossStateTreeTaskInstanceData;

	FPBBossStateTreeTask();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	void ExecuteAction(APBBossBase& Boss, EPBBossStateTreeTaskAction Action) const;

	TStateTreeExternalDataHandle<APBBossBase> BossHandle;
};
