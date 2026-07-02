#include "PBBossDamageComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossWeaknessComponent.h"

UPBBossDamageComponent::UPBBossDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APBBossBase>(GetOwner());
}

void UPBBossDamageComponent::ApplyHitPartDamage(
	AActor* DamageSource,
	UPrimitiveComponent* HitComponent,
	int32 DamageAmount,
	const FHitResult& Hit)
{
	ApplyResolvedDamage(DamageSource, ResolveHitPartInfo(HitComponent, Hit), DamageAmount, Hit);
}

void UPBBossDamageComponent::SetPinballCollisionDamageBlocked(bool IsBlocked)
{
	IsPinballCollisionDamageBlockedValue = IsBlocked;
}

bool UPBBossDamageComponent::IsPinballCollisionDamageBlocked() const
{
	return IsPinballCollisionDamageBlockedValue;
}

UPBBossDamageComponent::FPBBossHitPartInfo UPBBossDamageComponent::ResolveHitPartInfo(const UPrimitiveComponent* HitComponent, const FHitResult& Hit) const
{
	if (const UPBBossHitPartComponent* HitPartComponent = FindHitPartComponent(HitComponent))
	{
		FPBBossHitPartInfo HitPartInfo;
		HitPartInfo.HitPartType = HitPartComponent->GetHitPartType();
		HitPartInfo.HitPointName = HitPartComponent->GetHitPointName();
		return HitPartInfo;
	}

	if (Hit.BoneName != NAME_None)
	{
		if (const UPBBossHitPartComponent* HitPartComponent = FindHitPartComponent(Hit.BoneName))
		{
			FPBBossHitPartInfo HitPartInfo;
			HitPartInfo.HitPartType = HitPartComponent->GetHitPartType();
			HitPartInfo.HitPointName = HitPartComponent->GetHitPointName();
			return HitPartInfo;
		}
	}

	FPBBossHitPartInfo HitPartInfo;
	HitPartInfo.HitPartType = EPBBossHitPartType::Body;
	HitPartInfo.HitPointName = Hit.BoneName != NAME_None
		? Hit.BoneName
		: DefaultHitPointName;

	if (OwnerBoss && OwnerBoss->GetBossWeaknessComponent() && OwnerBoss->GetBossWeaknessComponent()->IsWeaknessPoint(HitPartInfo.HitPointName))
	{
		HitPartInfo.HitPartType = EPBBossHitPartType::WeakPoint;
	}

	return HitPartInfo;
}

const UPBBossHitPartComponent* UPBBossDamageComponent::FindHitPartComponent(const UPrimitiveComponent* HitComponent) const
{
	if (!OwnerBoss || !HitComponent)
	{
		return nullptr;
	}

	TArray<UPBBossHitPartComponent*> HitPartComponents;
	OwnerBoss->GetComponents<UPBBossHitPartComponent>(HitPartComponents);

	for (const UPBBossHitPartComponent* HitPartComponent : HitPartComponents)
	{
		if (HitPartComponent && HitPartComponent->IsTargetHitComponent(HitComponent))
		{
			return HitPartComponent;
		}
	}

	return nullptr;
}

const UPBBossHitPartComponent* UPBBossDamageComponent::FindHitPartComponent(FName HitPointName) const
{
	if (!OwnerBoss || HitPointName == NAME_None)
	{
		return nullptr;
	}

	TArray<UPBBossHitPartComponent*> HitPartComponents;
	OwnerBoss->GetComponents<UPBBossHitPartComponent>(HitPartComponents);

	for (const UPBBossHitPartComponent* HitPartComponent : HitPartComponents)
	{
		if (HitPartComponent && HitPartComponent->GetHitPointName() == HitPointName)
		{
			return HitPartComponent;
		}
	}

	return nullptr;
}

bool UPBBossDamageComponent::CanApplyDamage(FName HitPointName, int32 DamageAmount) const
{
	return OwnerBoss && DamageAmount > 0 && !OwnerBoss->IsDead() && HitPointName != NAME_None;
}

bool UPBBossDamageComponent::IsDamageBlocked(AActor* DamageSource) const
{
	return IsPinballCollisionDamageBlockedValue && Cast<APBBallBase>(DamageSource);
}

bool UPBBossDamageComponent::CanApplyDamageRateLimit() const
{
	if (MaxDamageCountPerFrame <= 0)
	{
		return false;
	}

	if (LastDamageFrameNumber == GFrameCounter && CurrentFrameDamageCount >= MaxDamageCountPerFrame)
	{
		return false;
	}

	if (DamageCooldownSeconds <= 0.0f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	if (LastDamageTimeSeconds < 0.0f)
	{
		return true;
	}

	return World->GetTimeSeconds() - LastDamageTimeSeconds >= DamageCooldownSeconds;
}

bool UPBBossDamageComponent::IsWeakPointHitBlocked(const FPBBossHitPartInfo& HitPartInfo) const
{
	if (HitPartInfo.HitPartType != EPBBossHitPartType::WeakPoint)
	{
		return false;
	}

	const UPBBossWeaknessComponent* WeaknessComponent = OwnerBoss ? OwnerBoss->GetBossWeaknessComponent() : nullptr;
	return WeaknessComponent && !WeaknessComponent->IsWeaknessPointOpen(HitPartInfo.HitPointName);
}

void UPBBossDamageComponent::ApplyResolvedDamage(
	AActor* DamageSource,
	const FPBBossHitPartInfo& HitPartInfo,
	int32 DamageAmount,
	const FHitResult& Hit)
{
	if (IsDamageBlocked(DamageSource)
		|| !CanApplyDamage(HitPartInfo.HitPointName, DamageAmount)
		|| !CanApplyDamageRateLimit()
		|| IsWeakPointHitBlocked(HitPartInfo))
	{
		return;
	}

	ApplyDamageToBoss(HitPartInfo.HitPointName, DamageAmount);
	UE_LOG(LogTemp, Warning, TEXT("Boss Damage Resolved: %s, Type %d, Damage %d"),
		*HitPartInfo.HitPointName.ToString(),
		static_cast<int32>(HitPartInfo.HitPartType),
		DamageAmount);
	RecordDamageRateLimit();
	OnDamageSourceHitApplied.Broadcast(DamageSource, Hit);
}

void UPBBossDamageComponent::ApplyDamageToBoss(FName HitPointName, int32 DamageAmount)
{
	if (!OwnerBoss)
	{
		return;
	}

	OwnerBoss->NotifyBossDamaged(HitPointName, DamageAmount);

	if (UPBBossStatComponent* StatComponent = OwnerBoss->GetBossStatComponent())
	{
		StatComponent->ApplyBossDamage(HitPointName, DamageAmount);
	}
}

void UPBBossDamageComponent::RecordDamageRateLimit()
{
	if (LastDamageFrameNumber != GFrameCounter)
	{
		LastDamageFrameNumber = GFrameCounter;
		CurrentFrameDamageCount = 0;
	}

	++CurrentFrameDamageCount;

	if (DamageCooldownSeconds <= 0.0f)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		LastDamageTimeSeconds = World->GetTimeSeconds();
	}
}
