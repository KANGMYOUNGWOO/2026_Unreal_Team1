#include "PBBossStatComponent.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Utils/PBFixedPoint.h"

UPBBossStatComponent::UPBBossStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossStatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
	MaxHP = FMath::Max(MaxHP, 1);
	MaxHPRaw = FPBFixedPoint::ToRawNonNegative(static_cast<float>(MaxHP));
	HPRaw = MaxHPRaw;
	RefreshDisplayedHP();
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
	const int64 DamageRaw = static_cast<int64>(FPBFixedPoint::ToRaw(static_cast<float>(DamageAmount))) * HPDamageMultiplierPercent / 100;
	const int32 ClampedDamageRaw = static_cast<int32>(FMath::Min<int64>(DamageRaw, TNumericLimits<int32>::Max()));
	const int32 PreviousHP = HP;

	HPRaw = FPBFixedPoint::ClampRaw(HPRaw - ClampedDamageRaw, 0, MaxHPRaw);
	RefreshDisplayedHP();
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
	return HPRaw <= 0;
}

int32 UPBBossStatComponent::GetHPDamageMultiplierPercent(FName HitPointName) const
{
	if (const FBossHitPointDamageData* HitPointData = HitPointDataMap.Find(HitPointName))
	{
		return HitPointData->HPDamageMultiplierPercent;
	}

	return DefaultHPDamageMultiplierPercent;
}

int32 UPBBossStatComponent::GetDisplayedHP() const
{
	return FMath::FloorToInt(FPBFixedPoint::ToFloat(HPRaw));
}

void UPBBossStatComponent::RefreshDisplayedHP()
{
	MaxHP = FMath::Max(1, FMath::FloorToInt(FPBFixedPoint::ToFloat(MaxHPRaw)));
	HP = FMath::Clamp(GetDisplayedHP(), 0, MaxHP);
}

bool UPBBossStatComponent::IsEnrageThresholdReached() const
{
	return static_cast<int64>(HPRaw) * 100 <= static_cast<int64>(MaxHPRaw) * EnrageHPRatioPercent;
}

bool UPBBossStatComponent::CanNotifyOwner() const
{
	return OwnerActor && OwnerActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass());
}
