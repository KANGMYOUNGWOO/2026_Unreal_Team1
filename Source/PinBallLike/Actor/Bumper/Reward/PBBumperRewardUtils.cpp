#include "PBBumperRewardUtils.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperReactiveRepairComponent.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperTimedStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

namespace
{
	constexpr float MaxResourceRewardPerApplication = 100000.0f;
	constexpr int32 MaxDiscreteRewardApplications = 100;
}

FPBBumperRewardApplyResult PBBumperRewardUtils::ApplyReward(
	AActor* TargetActor,
	const EPBBumperRewardType RewardType,
	const FName ResourceName,
	const FName StatusEffectId,
	const float RequestedPower,
	const FName TimedEffectSourceId,
	const float TimedEffectDuration,
	const FName TimedStatName,
	const int32 TriggerCount)
{
	FPBBumperRewardApplyResult Result;
	if (!IsValid(TargetActor) || !FMath::IsFinite(RequestedPower) || RequestedPower <= 0.0f)
	{
		return Result;
	}

	switch (RewardType)
	{
	case EPBBumperRewardType::Resource:
	{
		UPBBaseResourceComponent* ResourceComponent =
			TargetActor->FindComponentByClass<UPBBaseResourceComponent>();
		if (!IsValid(ResourceComponent)
			|| ResourceComponent->IsDead()
			|| ResourceName.IsNone()
			|| !ResourceComponent->HasResource(ResourceName))
		{
			return Result;
		}

		const float AppliedPower = FMath::Min(RequestedPower, MaxResourceRewardPerApplication);
		const float PreviousValue = ResourceComponent->GetResourceCurrent(ResourceName);
		ResourceComponent->ApplyResourceDelta(ResourceName, AppliedPower);
		Result.AppliedValue = ResourceComponent->GetResourceCurrent(ResourceName) - PreviousValue;
		Result.bApplied = Result.AppliedValue > KINDA_SMALL_NUMBER;
		Result.AppliedCount = Result.bApplied ? 1 : 0;
		return Result;
	}

	case EPBBumperRewardType::Combo:
	{
		IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(TargetActor);
		if (!Comboable)
		{
			return Result;
		}

		const int32 ComboAmount = FMath::Clamp(
			FMath::RoundToInt(RequestedPower),
			1,
			MaxDiscreteRewardApplications);
		const int32 PreviousCombo = Comboable->GetCombo();
		Comboable->AddCombo(ComboAmount);
		Result.AppliedValue = static_cast<float>(Comboable->GetCombo() - PreviousCombo);
		Result.bApplied = Result.AppliedValue > 0.0f;
		Result.AppliedCount = Result.bApplied ? ComboAmount : 0;
		return Result;
	}

	case EPBBumperRewardType::StatusEffect:
	{
		UPBStatusEffectComponent* StatusEffectComponent =
			TargetActor->FindComponentByClass<UPBStatusEffectComponent>();
		if (!IsValid(StatusEffectComponent) || StatusEffectId.IsNone())
		{
			return Result;
		}

		const int32 ApplicationCount = FMath::Clamp(
			FMath::RoundToInt(RequestedPower),
			1,
			MaxDiscreteRewardApplications);
		for (int32 Index = 0; Index < ApplicationCount; ++Index)
		{
			if (!StatusEffectComponent->ApplyStatusEffect(StatusEffectId))
			{
				break;
			}

			++Result.AppliedCount;
		}

		Result.AppliedValue = static_cast<float>(Result.AppliedCount);
		Result.bApplied = Result.AppliedCount > 0;
		return Result;
	}

	case EPBBumperRewardType::TimedAttackPercent:
	case EPBBumperRewardType::TimedStatPercent:
	{
		if (TimedEffectSourceId.IsNone()
			|| !FMath::IsFinite(TimedEffectDuration)
			|| TimedEffectDuration <= 0.0f)
		{
			return Result;
		}

		UPBBumperTimedStatComponent* TimedStatComponent =
			TargetActor->FindComponentByClass<UPBBumperTimedStatComponent>();
		if (!IsValid(TimedStatComponent))
		{
			TimedStatComponent = NewObject<UPBBumperTimedStatComponent>(
				TargetActor,
				UPBBumperTimedStatComponent::StaticClass(),
				NAME_None,
				RF_Transient);
			if (IsValid(TimedStatComponent))
			{
				TargetActor->AddInstanceComponent(TimedStatComponent);
				TimedStatComponent->RegisterComponent();
			}
		}

		int32 SourceBonus = 0;
		const FName StatName = RewardType == EPBBumperRewardType::TimedAttackPercent
			? PBStatNames::Attack
			: TimedStatName;
		Result.bApplied = IsValid(TimedStatComponent)
			&& TimedStatComponent->ApplyTimedStatPercent(
				TimedEffectSourceId,
				StatName,
				RequestedPower,
				TimedEffectDuration,
				SourceBonus);
		Result.AppliedValue = static_cast<float>(SourceBonus);
		Result.AppliedCount = Result.bApplied ? 1 : 0;
		return Result;
	}

	case EPBBumperRewardType::PostDamageHeal:
	{
		if (ResourceName.IsNone()
			|| TriggerCount <= 0
			|| !FMath::IsFinite(TimedEffectDuration)
			|| TimedEffectDuration <= 0.0f)
		{
			return Result;
		}

		UPBBumperReactiveRepairComponent* RepairComponent =
			TargetActor->FindComponentByClass<UPBBumperReactiveRepairComponent>();
		if (!IsValid(RepairComponent))
		{
			RepairComponent = NewObject<UPBBumperReactiveRepairComponent>(
				TargetActor,
				UPBBumperReactiveRepairComponent::StaticClass(),
				NAME_None,
				RF_Transient);
			if (IsValid(RepairComponent))
			{
				TargetActor->AddInstanceComponent(RepairComponent);
				RepairComponent->RegisterComponent();
			}
		}

		Result.bApplied = IsValid(RepairComponent)
			&& RepairComponent->Arm(
				ResourceName,
				TriggerCount,
				RequestedPower,
				TimedEffectDuration);
		Result.AppliedValue = Result.bApplied ? RequestedPower : 0.0f;
		Result.AppliedCount = Result.bApplied ? TriggerCount : 0;
		return Result;
	}

	default:
		return Result;
	}
}
