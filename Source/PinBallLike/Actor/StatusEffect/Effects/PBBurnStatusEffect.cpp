#include "PinBallLike/Actor/StatusEffect/Effects/PBBurnStatusEffect.h"

#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

void UPBBurnStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
	if (TriggerEvent != PBStatusEffectTriggerEvents::Interval)
	{
		return;
	}

	AActor* OwnerActor = GetOwnerActor();
	IDamageable* Damageable = Cast<IDamageable>(OwnerActor);
	if (!Damageable)
	{
		Damageable = PBInterfaceUtils::FindInterface<IDamageable>(OwnerActor);
	}

	if (!Damageable || Damageable->IsDead())
	{
		return;
	}

	const int32 Damage = CalculateDamage();
	if (Damage > 0)
	{
		Damageable->TakeDamage(Damage);
	}
}

int32 UPBBurnStatusEffect::CalculateDamage() const
{
	float TotalDamage = 0.0f;
	for (const FPBStatusEffectTriggerRow& TriggerRow : TriggerRows)
	{
		const float Scale = TriggerRow.bScaleWithStack ? static_cast<float>(GetStackCount()) : 1.0f;
		TotalDamage += TriggerRow.Value * Scale;
	}

	return FMath::Max(0, FMath::RoundToInt(TotalDamage));
}
