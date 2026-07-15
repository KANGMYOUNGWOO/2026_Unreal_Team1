#include "PinBallLike/Actor/StatusEffect/Effects/PBStrengthStatusEffect.h"

#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"

void UPBStrengthStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
	if (TriggerEvent == PBStatusEffectTriggerEvents::Applied
		|| TriggerEvent == PBStatusEffectTriggerEvents::Stacked)
	{
		const int32 NewAttackBonus = CalculateAttackBonus();
		ApplyAttackDelta(NewAttackBonus - AppliedAttackBonus);
		AppliedAttackBonus = NewAttackBonus;
		return;
	}

	if (TriggerEvent == PBStatusEffectTriggerEvents::Removed
		|| TriggerEvent == PBStatusEffectTriggerEvents::Expired)
	{
		ApplyAttackDelta(-AppliedAttackBonus);
		AppliedAttackBonus = 0;
	}
}

int32 UPBStrengthStatusEffect::CalculateAttackBonus() const
{
	float AttackBonus = 0.0f;
	for (const FPBStatusEffectModifierRow& ModifierRow : ModifierRows)
	{
		const float Scale = ModifierRow.bScaleWithStack
			? static_cast<float>(GetStackCount())
			: 1.0f;
		AttackBonus += ModifierRow.Value * Scale;
	}

	return FMath::Max(0, FMath::RoundToInt(AttackBonus));
}

void UPBStrengthStatusEffect::ApplyAttackDelta(const int32 Delta) const
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
	StatComponent->ApplyStat(PBStatNames::Attack, Delta);
}
