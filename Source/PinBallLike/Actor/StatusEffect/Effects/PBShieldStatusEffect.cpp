#include "PinBallLike/Actor/StatusEffect/Effects/PBShieldStatusEffect.h"

#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"

void UPBShieldStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
	if (TriggerEvent == PBStatusEffectTriggerEvents::Applied
		|| TriggerEvent == PBStatusEffectTriggerEvents::Stacked)
	{
		const float NewShieldValue = CalculateShieldValue();
		AppliedShieldValue += ApplyShieldDelta(NewShieldValue - AppliedShieldValue);
		return;
	}

	if (TriggerEvent == PBStatusEffectTriggerEvents::Removed
		|| TriggerEvent == PBStatusEffectTriggerEvents::Expired)
	{
		ApplyShieldDelta(-AppliedShieldValue);
		AppliedShieldValue = 0.0f;
	}
}

float UPBShieldStatusEffect::CalculateShieldValue() const
{
	float ShieldValue = 0.0f;
	for (const FPBStatusEffectModifierRow& ModifierRow : ModifierRows)
	{
		const float Scale = ModifierRow.bScaleWithStack ? static_cast<float>(GetStackCount()) : 1.0f;
		ShieldValue += ModifierRow.Value * Scale;
	}

	return FMath::Max(0.0f, ShieldValue);
}

float UPBShieldStatusEffect::ApplyShieldDelta(const float Delta) const
{
	AActor* OwnerActor = GetOwnerActor();
	if (!IsValid(OwnerActor) || FMath::IsNearlyZero(Delta))
	{
		return 0.0f;
	}

	UPBBaseResourceComponent* ResourceComponent = OwnerActor->FindComponentByClass<UPBBaseResourceComponent>();
	if (!ResourceComponent)
	{
		return 0.0f;
	}

	if (!ResourceComponent->HasResource(PBResourceNames::Shield))
	{
		if (Delta <= 0.0f)
		{
			return 0.0f;
		}

		ResourceComponent->SetResource(PBResourceNames::Shield, 0.0f, Delta);
	}
	else if (Delta > 0.0f)
	{
		const float CurrentShield = ResourceComponent->GetResourceCurrent(PBResourceNames::Shield);
		const float MaxShield = ResourceComponent->GetResourceMax(PBResourceNames::Shield);
		const float RequiredMaxShield = CurrentShield + Delta;
		if (RequiredMaxShield > MaxShield)
		{
			ResourceComponent->SetResourceMax(PBResourceNames::Shield, RequiredMaxShield, false);
		}
	}

	const float PreviousShield = ResourceComponent->GetResourceCurrent(PBResourceNames::Shield);
	ResourceComponent->ApplyResourceDelta(PBResourceNames::Shield, Delta);
	return ResourceComponent->GetResourceCurrent(PBResourceNames::Shield) - PreviousShield;
}
