#include "PBBallEffectRuntimeComponent.h"

#include "PBBallResourceComponent.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Effect/Runtime/PBEffectRuntimeInstance.h"

UPBBallEffectRuntimeComponent::UPBBallEffectRuntimeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallEffectRuntimeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (UPBBallResourceComponent* ResourceComponent = Owner->FindComponentByClass<UPBBallResourceComponent>())
		{
			ResourceComponent->OnResourceRevived.AddUObject(this, &UPBBallEffectRuntimeComponent::HandleResourceRevived);
		}
	}
}

void UPBBallEffectRuntimeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		if (UPBBallResourceComponent* ResourceComponent = Owner->FindComponentByClass<UPBBallResourceComponent>())
		{
			ResourceComponent->OnResourceRevived.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPBBallEffectRuntimeComponent::AddFirstAttackRule(const float AttackPercent, const int32 ExtraDamage)
{
	UPBFirstAttackEffectRuntimeInstance* Instance = NewObject<UPBFirstAttackEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(AttackPercent, ExtraDamage);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddComboStatBuffRule(
	const FName StatName,
	const FName ModifyType,
	const float Value,
	const int32 RequiredCombo)
{
	if (StatName.IsNone() || ModifyType.IsNone() || RequiredCombo <= 0)
	{
		return;
	}

	UPBComboStatBuffEffectRuntimeInstance* Instance = NewObject<UPBComboStatBuffEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(StatName, ModifyType, Value, RequiredCombo);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddComboPeriodStatBuffRule(
	FName StatName, 
	FName ModifyType, 
	float Value,
	int32 RequiredCombo)
{
	if (StatName.IsNone() || ModifyType.IsNone() || RequiredCombo <= 0)
	{
		return;
	}

	UPBComboPeriodStatBuffEffectRuntimeInstance* Instance =
		NewObject<UPBComboPeriodStatBuffEffectRuntimeInstance>(this);

	Instance->Initialize(this);
	Instance->Setup(
		StatName,
		ModifyType,
		Value,
		RequiredCombo);

	RegisterRuntimeInstance(Instance);
	

}

void UPBBallEffectRuntimeComponent::AddSkillDamagePercent(const float Percent)
{
	UPBSkillDamageEffectRuntimeInstance* Instance = NewObject<UPBSkillDamageEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(Percent);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddOnHitStatStackRule(
	const FName StatName,
	const FName ModifyType,
	const float Value)
{
	if (StatName.IsNone() || ModifyType.IsNone())
	{
		return;
	}

	UPBOnHitStatStackEffectRuntimeInstance* Instance = NewObject<UPBOnHitStatStackEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(StatName, ModifyType, Value);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddOnHitDamageIgnoreChance(
	const FName ResourceName,
	const int32 IgnoreCount,
	const float ChancePercent)
{
	UPBOnHitDamageIgnoreEffectRuntimeInstance* Instance = NewObject<UPBOnHitDamageIgnoreEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(ResourceName, IgnoreCount, ChancePercent);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddComboExtraDamageRule(const int32 ComboInterval, const int32 DamageAmount)
{
	UPBComboExtraDamageEffectRuntimeInstance* Instance = NewObject<UPBComboExtraDamageEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(ComboInterval, DamageAmount);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddPendingMultiBallCount(const int32 Count)
{
	PendingMultiBallCount += FMath::Max(0, Count);
}

void UPBBallEffectRuntimeComponent::AddPierceRule(const float DamageRetentionPercent)
{
	bPierceGranted = true;
	PierceDamageRetentionPercent = FMath::Max(PierceDamageRetentionPercent, DamageRetentionPercent);
}

void UPBBallEffectRuntimeComponent::AddAttackPercent(const float Percent)
{
	UPBAttackPercentEffectRuntimeInstance* Instance = NewObject<UPBAttackPercentEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(Percent);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddReviveStatBuffRule(
	const FName StatName,
	const FName ModifyType,
	const float Value)
{
	if (StatName.IsNone() || ModifyType.IsNone())
	{
		return;
	}

	UPBReviveStatBuffEffectRuntimeInstance* Instance = NewObject<UPBReviveStatBuffEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(StatName, ModifyType, Value);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddFirstHitTimedStatBuffRule(
	const FName StatName,
	const FName ModifyType,
	const float Value,
	const float Duration)
{
	if (StatName.IsNone() || ModifyType.IsNone())
	{
		return;
	}

	UPBFirstHitTimedStatBuffEffectRuntimeInstance* Instance = NewObject<UPBFirstHitTimedStatBuffEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(StatName, ModifyType, Value, Duration);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddSwitchNextHitDamageRule(const float AttackPercent)
{
	UPBSwitchNextHitDamageEffectRuntimeInstance* Instance = NewObject<UPBSwitchNextHitDamageEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(AttackPercent);
	RegisterRuntimeInstance(Instance);
}

void UPBBallEffectRuntimeComponent::AddSwitchTimedStatBuffRule(
	const FName StatName,
	const FName ModifyType,
	const float Value,
	const float Duration)
{
	if (StatName.IsNone() || ModifyType.IsNone())
	{
		return;
	}

	UPBSwitchTimedStatBuffEffectRuntimeInstance* Instance = NewObject<UPBSwitchTimedStatBuffEffectRuntimeInstance>(this);
	Instance->Initialize(this);
	Instance->Setup(StatName, ModifyType, Value, Duration);
	RegisterRuntimeInstance(Instance);
	Instance->ApplyNow();
}

int32 UPBBallEffectRuntimeComponent::ModifyCollisionDamage(const int32 BaseDamage)
{
	int32 Damage = BaseDamage;
	for (IPBCollisionDamageModifier* Modifier : CollisionDamageModifiers)
	{
		if (Modifier)
		{
			Damage = Modifier->ModifyCollisionDamage(Damage);
		}
	}

	return Damage;
}

int32 UPBBallEffectRuntimeComponent::ModifySkillDamage(const int32 BaseDamage) const
{
	int32 Damage = BaseDamage;
	for (const IPBSkillDamageModifier* Modifier : SkillDamageModifiers)
	{
		if (Modifier)
		{
			Damage = Modifier->ModifySkillDamage(Damage);
		}
	}

	return Damage;
}

void UPBBallEffectRuntimeComponent::HandleEnemyHit(AActor* EnemyActor)
{
	for (IPBEnemyHitListener* Listener : EnemyHitListeners)
	{
		if (Listener)
		{
			Listener->HandleEnemyHit(EnemyActor);
		}
	}
}

void UPBBallEffectRuntimeComponent::HandleComboChanged(const int32 CurrentCombo, const int32 MaxCombo)
{
	for (IPBComboChangedListener* Listener : ComboChangedListeners)
	{
		if (Listener)
		{
			Listener->HandleComboChanged(CurrentCombo, MaxCombo);
		}
	}
}

void UPBBallEffectRuntimeComponent::RegisterRuntimeInstance(UPBEffectRuntimeInstanceBase* Instance)
{
	if (!Instance)
	{
		return;
	}

	ActiveEffectInstances.Add(Instance);

	if (IPBCollisionDamageModifier* Modifier = Instance->AsCollisionDamageModifier())
	{
		CollisionDamageModifiers.Add(Modifier);
	}
	if (IPBSkillDamageModifier* Modifier = Instance->AsSkillDamageModifier())
	{
		SkillDamageModifiers.Add(Modifier);
	}
	if (IPBEnemyHitListener* Listener = Instance->AsEnemyHitListener())
	{
		EnemyHitListeners.Add(Listener);
	}
	if (IPBComboChangedListener* Listener = Instance->AsComboChangedListener())
	{
		ComboChangedListeners.Add(Listener);
	}
	if (IPBResourceRevivedListener* Listener = Instance->AsResourceRevivedListener())
	{
		ResourceRevivedListeners.Add(Listener);
	}
}

void UPBBallEffectRuntimeComponent::HandleResourceRevived(
	const FName ResourceName,
	const float ReviveValue)
{
	for (IPBResourceRevivedListener* Listener : ResourceRevivedListeners)
	{
		if (Listener)
		{
			Listener->HandleResourceRevived(ResourceName, ReviveValue);
		}
	}
}
