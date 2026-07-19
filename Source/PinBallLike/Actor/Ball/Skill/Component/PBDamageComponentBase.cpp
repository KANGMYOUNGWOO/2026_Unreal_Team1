#include "PBDamageComponentBase.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Ball/Component/PBBallEffectRuntimeComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

UPBDamageComponentBase::UPBDamageComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBDamageComponentBase::CanApplyDamage(AActor* Target, const int32 DamageAmount) const
{
	if (!IsValid(Target) || Target == GetOwner() || DamageAmount <= 0)
	{
		return false;
	}

	if (Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		return true;
	}

	IDamageable* Damageable = Cast<IDamageable>(Target);
	if (!Damageable)
	{
		Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Target);
	}

	return Damageable && !Damageable->IsDead();
}

bool UPBDamageComponentBase::ApplyDamage(AActor* Target, const int32 DamageAmount)
{
	AActor* SourceActor = GetOwner() ? GetOwner()->GetOwner() : nullptr;
	const UPBBallEffectRuntimeComponent* EffectRuntimeComponent = SourceActor
		? SourceActor->FindComponentByClass<UPBBallEffectRuntimeComponent>()
		: nullptr;
	const int32 FinalDamageAmount = EffectRuntimeComponent
		? EffectRuntimeComponent->ModifySkillDamage(DamageAmount)
		: DamageAmount;

	if (!CanApplyDamage(Target, FinalDamageAmount))
	{
		return false;
	}

	if (Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		if (!IBossInterface::Execute_DamageToBoss(Target, FinalDamageAmount))
		{
			return false;
		}
	}
	else
	{
		IDamageable* Damageable = Cast<IDamageable>(Target);
		if (!Damageable)
		{
			Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Target);
		}

		if (!Damageable)
		{
			return false;
		}

		Damageable->TakeDamage(FinalDamageAmount);
	}

	OnDamageApplied.Broadcast(Target, FinalDamageAmount);
	return true;
}
