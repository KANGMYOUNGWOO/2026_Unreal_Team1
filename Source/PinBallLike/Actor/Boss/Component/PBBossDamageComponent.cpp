#include "PBBossDamageComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossWeaknessComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"

UPBBossDamageComponent::UPBBossDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APBBossBase>(GetOwner());
}

void UPBBossDamageComponent::DamageToBose(AActor* DamageSource, int32 DamageAmount)
{
	FPBBossHitPartInfo HitPartInfo;
	HitPartInfo.HitPartType = EPBBossHitPartType::Body;
	HitPartInfo.HitPointName = DefaultHitPointName;

	ApplyResolvedDamage(DamageSource, HitPartInfo, DamageAmount, FHitResult());
}

void UPBBossDamageComponent::ApplyPointDamage(FName HitPointName, int32 DamageAmount)
{
	FPBBossHitPartInfo HitPartInfo;
	HitPartInfo.HitPartType = EPBBossHitPartType::Body;
	HitPartInfo.HitPointName = HitPointName.IsNone() ? DefaultHitPointName : HitPointName;

	if (OwnerBoss && OwnerBoss->GetBossWeaknessComponent() && OwnerBoss->GetBossWeaknessComponent()->IsWeaknessPoint(HitPartInfo.HitPointName))
	{
		HitPartInfo.HitPartType = EPBBossHitPartType::WeakPoint;
	}

	if (!CanApplyDamage(HitPartInfo.HitPointName, DamageAmount) || IsWeakPointHitBlocked(HitPartInfo))
	{
		return;
	}

	int32 FinalDamageAmount = DamageAmount;
	if (UPBBossWeaknessComponent* WeaknessComponent = OwnerBoss->GetBossWeaknessComponent())
	{
		FinalDamageAmount = WeaknessComponent->CalculateWeaknessDamage(HitPartInfo.HitPointName, DamageAmount);
	}

	BroadcastDamageApplied(HitPartInfo.HitPointName, FinalDamageAmount);
}

void UPBBossDamageComponent::ApplyHitPartDamage(
	AActor* DamageSource,
	UPrimitiveComponent* HitComponent,
	int32 DamageAmount,
	const FHitResult& Hit)
{
	ApplyResolvedDamage(DamageSource, ResolveHitPartInfo(HitComponent), DamageAmount, Hit);
}

void UPBBossDamageComponent::SetPinballCollisionDamageBlocked(bool IsBlocked)
{
	IsPinballCollisionDamageBlockedValue = IsBlocked;
}

bool UPBBossDamageComponent::IsPinballCollisionDamageBlocked() const
{
	return IsPinballCollisionDamageBlockedValue;
}

bool UPBBossDamageComponent::IsValidDamageSource(AActor* DamageSource, UPrimitiveComponent* DamageSourceComponent) const
{
	if (!DamageSource)
	{
		return false;
	}

	if (Cast<APBBallBase>(DamageSource))
	{
		return true;
	}

	if (DamageSourceTagName.IsNone())
	{
		return false;
	}

	const bool IsActorTagged = DamageSource->ActorHasTag(DamageSourceTagName);
	const bool IsComponentTagged = DamageSourceComponent && DamageSourceComponent->ComponentHasTag(DamageSourceTagName);

	return IsActorTagged || IsComponentTagged;
}

int32 UPBBossDamageComponent::GetPinballHitDamage(AActor* DamageSource) const
{
	if (IsPinballCollisionDamageBlocked())
	{
		return 0;
	}

	if (!DamageSource)
	{
		return 0;
	}

	const UPBBaseStatComponent* StatComponent = DamageSource->FindComponentByClass<UPBBaseStatComponent>();
	return StatComponent ? StatComponent->GetStat(PBStatNames::Attack) : 0;
}

UPBBossDamageComponent::FPBBossHitPartInfo UPBBossDamageComponent::ResolveHitPartInfo(const UPrimitiveComponent* HitComponent) const
{
	if (const UPBBossHitPartComponent* HitPartComponent = FindHitPartComponent(HitComponent))
	{
		FPBBossHitPartInfo HitPartInfo;
		HitPartInfo.HitPartType = HitPartComponent->GetHitPartType();
		HitPartInfo.HitPointName = HitPartComponent->GetHitPointName();
		return HitPartInfo;
	}

	FPBBossHitPartInfo HitPartInfo;
	HitPartInfo.HitPartType = EPBBossHitPartType::Body;
	HitPartInfo.HitPointName = HitComponent && !HitComponent->ComponentTags.IsEmpty()
		? HitComponent->ComponentTags[0]
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

bool UPBBossDamageComponent::CanApplyDamage(FName HitPointName, int32 DamageAmount) const
{
	return OwnerBoss && DamageAmount > 0 && !OwnerBoss->IsDead() && HitPointName != NAME_None;
}

bool UPBBossDamageComponent::CanApplyDamageRateLimit(AActor* DamageSource) const
{
	if (MaxDamageCountPerFrame <= 0)
	{
		return false;
	}

	if (LastDamageFrameNumber == GFrameCounter && CurrentFrameDamageCount >= MaxDamageCountPerFrame)
	{
		return false;
	}

	if (!DamageSource || SameSourceHitCooldownSeconds <= 0.0f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const float* LastDamageTime = LastDamageTimeMap.Find(TObjectKey<AActor>(DamageSource));
	if (!LastDamageTime)
	{
		return true;
	}

	return World->GetTimeSeconds() - *LastDamageTime >= SameSourceHitCooldownSeconds;
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
	if (!CanApplyDamage(HitPartInfo.HitPointName, DamageAmount)
		|| !CanApplyDamageRateLimit(DamageSource)
		|| IsWeakPointHitBlocked(HitPartInfo))
	{
		return;
	}

	int32 FinalDamageAmount = DamageAmount;
	if (UPBBossWeaknessComponent* WeaknessComponent = OwnerBoss->GetBossWeaknessComponent())
	{
		FinalDamageAmount = WeaknessComponent->CalculateWeaknessDamage(HitPartInfo.HitPointName, DamageAmount);
	}

	BroadcastDamageApplied(HitPartInfo.HitPointName, FinalDamageAmount);

	RecordDamageRateLimit(DamageSource);
	OnDamageSourceHitApplied.Broadcast(DamageSource, Hit);
}

void UPBBossDamageComponent::BroadcastDamageApplied(FName HitPointName, int32 DamageAmount)
{
	OnBossDamageApplied.Broadcast(HitPointName, DamageAmount);
}

void UPBBossDamageComponent::RecordDamageRateLimit(AActor* DamageSource)
{
	if (LastDamageFrameNumber != GFrameCounter)
	{
		LastDamageFrameNumber = GFrameCounter;
		CurrentFrameDamageCount = 0;
	}

	++CurrentFrameDamageCount;

	if (!DamageSource || SameSourceHitCooldownSeconds <= 0.0f)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		LastDamageTimeMap.FindOrAdd(TObjectKey<AActor>(DamageSource)) = World->GetTimeSeconds();
	}
}
