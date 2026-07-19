#include "PinBallLike/Actor/StatusEffect/Effects/PBBounceStatusEffect.h"

#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"

void UPBBounceStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
	if (TriggerEvent == PBStatusEffectTriggerEvents::Applied
		|| TriggerEvent == PBStatusEffectTriggerEvents::Stacked)
	{
		const int32 NewBouncinessBonus = CalculateBouncinessBonus();
		ApplyBouncinessDelta(NewBouncinessBonus - AppliedBouncinessBonus);
		AppliedBouncinessBonus = NewBouncinessBonus;
		return;
	}

	if (TriggerEvent == PBStatusEffectTriggerEvents::Removed
		|| TriggerEvent == PBStatusEffectTriggerEvents::Expired)
	{
		ApplyBouncinessDelta(-AppliedBouncinessBonus);
		AppliedBouncinessBonus = 0;
	}
}

int32 UPBBounceStatusEffect::CalculateBouncinessBonus() const
{
	float BouncinessBonus = 0.0f;
	for (const FPBStatusEffectModifierRow& ModifierRow : ModifierRows)
	{
		const float Scale = ModifierRow.bScaleWithStack
			? static_cast<float>(GetStackCount())
			: 1.0f;
		BouncinessBonus += ModifierRow.Value * Scale;
	}

	return FMath::Max(0, FMath::RoundToInt(BouncinessBonus));
}

void UPBBounceStatusEffect::ApplyBouncinessDelta(const int32 Delta) const
{
	AActor* OwnerActor = GetOwnerActor();
	if (!IsValid(OwnerActor) || Delta == 0)
	{
		return;
	}

	UPBBaseStatComponent* StatComponent = OwnerActor->FindComponentByClass<UPBBaseStatComponent>();
	if (!StatComponent)
	{
		return;
	}

	// TODO: Replace direct stat mutation after the status effect modifier policy is finalized.
	StatComponent->ApplyStat(PBStatNames::Bounciness, Delta);
}
