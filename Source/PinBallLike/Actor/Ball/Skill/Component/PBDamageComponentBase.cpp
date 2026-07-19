#include "PBDamageComponentBase.h"

#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

UPBDamageComponentBase::UPBDamageComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBDamageComponentBase::SetGroggyAmount(const int32 InGroggyAmount)
{
	GroggyAmount = FMath::Max(InGroggyAmount, 0);
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
	if (!CanApplyDamage(Target, DamageAmount))
	{
		return false;
	}

	if (Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		if (!IBossInterface::Execute_DamageToBoss(Target, DamageAmount))
		{
			return false;
		}

		if (GroggyAmount > 0)
		{
			IBossInterface::Execute_IncreaseGroggy(Target, GroggyAmount);
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

		Damageable->TakeDamage(DamageAmount);
	}

	OnDamageApplied.Broadcast(Target, DamageAmount);
	return true;
}
