#include "PBDamageComponentBase.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Ball/Component/PBBallEffectRuntimeComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Interface/Comboable.h"
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

		Damageable->TakeDamage(FinalDamageAmount);
	}

	AddComboForSkillDamage();
	OnDamageApplied.Broadcast(Target, FinalDamageAmount);
	return true;
}

void UPBDamageComponentBase::AddComboForSkillDamage() const
{
	TSet<TWeakObjectPtr<AActor>> VisitedActors;
	AActor* SourceActor = GetOwner();
	while (IsValid(SourceActor))
	{
		const TWeakObjectPtr<AActor> SourceActorKey(SourceActor);
		if (VisitedActors.Contains(SourceActorKey))
		{
			return;
		}
		VisitedActors.Add(SourceActorKey);

		if (IComboable* Comboable =
			PBInterfaceUtils::FindInterface<IComboable>(SourceActor))
		{
			Comboable->AddCombo(1);
			return;
		}

		SourceActor = SourceActor->GetOwner();
	}
}
