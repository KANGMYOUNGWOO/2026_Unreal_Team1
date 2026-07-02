#include "PBBossDamageComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossWeaknessComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

UPBBossDamageComponent::UPBBossDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APBBossBase>(GetOwner());
}

void UPBBossDamageComponent::DamageToBoss(AActor* DamageSource, int32 DamageAmount)
{
	FPBBossHitPartInfo HitPartInfo;
	HitPartInfo.HitPartType = EPBBossHitPartType::Body;
	HitPartInfo.HitPointName = DefaultHitPointName;

	ApplyResolvedDamage(DamageSource, HitPartInfo, DamageAmount, FHitResult());
}

void UPBBossDamageComponent::DamageToBose(AActor* DamageSource, int32 DamageAmount)
{
	DamageToBoss(DamageSource, DamageAmount);
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

	ApplyResolvedDamage(nullptr, HitPartInfo, DamageAmount, FHitResult());
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

void UPBBossDamageComponent::HandleCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!IsValidDamageSource(OtherActor, OtherComponent))
	{
		return;
	}

	const int32 DamageAmount = GetPinballHitDamage(OtherActor);
	ApplyHitPartDamage(OtherActor, HitComponent, DamageAmount, Hit);
	OnDamageSourceHitApplied.Broadcast(OtherActor, Hit);
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
	if (!CanApplyDamage(HitPartInfo.HitPointName, DamageAmount)
		|| !CanApplyDamageRateLimit()
		|| IsWeakPointHitBlocked(HitPartInfo))
	{
		return;
	}

	ApplyDamageToBoss(HitPartInfo.HitPointName, DamageAmount);

	RecordDamageRateLimit();
	ApplyPinballHitImpulse(DamageSource, Hit);
	AddPinballCombo(DamageSource);
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

	if (!OwnerBoss->IsDead())
	{
		if (UPBBossGroggyComponent* GroggyComponent = OwnerBoss->GetBossGroggyComponent())
		{
			GroggyComponent->ApplyGroggyDamage(HitPointName);
		}
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

void UPBBossDamageComponent::ApplyPinballHitImpulse(AActor* DamageSource, const FHitResult& Hit) const
{
	if (!DamageSource || PinballHitImpulseStrength <= 0.0f || !OwnerBoss)
	{
		return;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(DamageSource);
	if (!Movable)
	{
		return;
	}

	FVector ImpulseDirection = DamageSource->GetActorLocation() - OwnerBoss->GetActorLocation();
	ImpulseDirection.Z = 0.0f;

	if (ImpulseDirection.IsNearlyZero())
	{
		ImpulseDirection = Hit.ImpactNormal;
		ImpulseDirection.Z = 0.0f;
	}

	ImpulseDirection = ImpulseDirection.GetSafeNormal();
	if (ImpulseDirection.IsNearlyZero())
	{
		return;
	}

	Movable->AddImpulse(ImpulseDirection * PinballHitImpulseStrength);
}

void UPBBossDamageComponent::AddPinballCombo(AActor* DamageSource) const
{
	IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(DamageSource);
	if (!Comboable)
	{
		return;
	}

	Comboable->AddCombo(1);
}
