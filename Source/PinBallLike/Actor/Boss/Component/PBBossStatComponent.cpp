#include "PBBossStatComponent.h"

#include "GameFramework/Actor.h"

UPBBossStatComponent::UPBBossStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossStatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
	MaxHP = FMath::Max(MaxHP, 1);
	HP = MaxHP;
	OnHPChanged.Broadcast(HP, MaxHP);

	if (!CanNotifyOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("BossStatComponent owner does not implement BossInterface."));
	}
}

void UPBBossStatComponent::ApplyBossDamage(FName HitPointName, int32 DamageAmount)
{
	if (DamageAmount <= 0 || IsDead())
	{
		return;
	}

	const int32 HPDamageMultiplierPercent = GetHPDamageMultiplierPercent(HitPointName);
	const int32 FinalHPDamage = DamageAmount * HPDamageMultiplierPercent / 100;
	const int32 PreviousHP = HP;

	HP = FMath::Clamp(HP - FinalHPDamage, 0, MaxHP);
	OnHPChanged.Broadcast(HP, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("Boss HP Damaged: %s, HP %d -> %d / %d"),
		*HitPointName.ToString(),
		PreviousHP,
		HP,
		MaxHP);

	if (!IsEnraged && IsEnrageThresholdReached() && !IsDead())
	{
		IsEnraged = true;
		UE_LOG(LogTemp, Warning, TEXT("Boss Enraged Triggered."));

		if (CanNotifyOwner())
		{
			IBossInterface::Execute_OnEnragedTriggered(OwnerActor);
		}
	}

	if (IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Dead."));

		if (CanNotifyOwner())
		{
			IBossInterface::Execute_OnDeadTriggered(OwnerActor);
		}
	}
}

bool UPBBossStatComponent::IsDead() const
{
	return HP <= 0;
}

int32 UPBBossStatComponent::GetHPDamageMultiplierPercent(FName HitPointName) const
{
	if (const FBossGroggyPointData* HitPointData = HitPointDataMap.Find(HitPointName))
	{
		return HitPointData->HPDamageMultiplierPercent;
	}

	return DefaultHPDamageMultiplierPercent;
}

bool UPBBossStatComponent::IsEnrageThresholdReached() const
{
	return HP * 100 <= MaxHP * EnrageHPRatioPercent;
}

bool UPBBossStatComponent::CanNotifyOwner() const
{
	return OwnerActor && OwnerActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass());
}
