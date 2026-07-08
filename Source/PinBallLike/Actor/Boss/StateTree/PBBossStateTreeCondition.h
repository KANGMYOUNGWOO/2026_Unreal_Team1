#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "StateTreeConditionBase.h"
#include "StateTreeExecutionTypes.h"
#include "PBBossStateTreeCondition.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeLinker;

UENUM(BlueprintType)
enum class EPBBossStateTreeConditionType : uint8
{
	Idle,
	Pattern,
	Groggy,
	Enraged,
	EnragedPhase,
	Dead,
	FixedBoss,
	MovableBoss
};

USTRUCT()
struct FPBBossStateTreeConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	EPBBossStateTreeConditionType ConditionType = EPBBossStateTreeConditionType::Idle;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool IsInvert = false;
};

USTRUCT(DisplayName = "Boss State Condition", meta = (Category = "Boss"))
struct FPBBossStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPBBossStateTreeConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<APBBossBase> BossHandle;
};
