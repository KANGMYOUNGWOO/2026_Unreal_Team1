#include "PBBossDamageComponent.h"

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
	BindHitPartCollisionEvents();
}

void UPBBossDamageComponent::ApplyHitPartDamage(int32 DamageAmount)
{
	const FName HitPointName = ResolveHitPointName();

	if (!CanApplyDamage(HitPointName, DamageAmount)
		|| IsPinballCollisionDamageBlockedValue
		|| !CanApplyDamageRateLimit()
		|| IsWeakPointHitBlocked(HitPointName))
	{
		return;
	}

	ApplyDamageToBoss(HitPointName, DamageAmount);
	UE_LOG(LogTemp, Warning, TEXT("Boss Damage Resolved: %s, Damage %d"),
		*HitPointName.ToString(),
		DamageAmount);
	RecordDamageRateLimit();
}

void UPBBossDamageComponent::SetPinballCollisionDamageBlocked(bool IsBlocked)
{
	IsPinballCollisionDamageBlockedValue = IsBlocked;
}

bool UPBBossDamageComponent::IsPinballCollisionDamageBlocked() const
{
	return IsPinballCollisionDamageBlockedValue;
}

bool UPBBossDamageComponent::CanApplyDamage(FName HitPointName, int32 DamageAmount) const
{
	return OwnerBoss && DamageAmount > 0 && !OwnerBoss->IsDead() && HitPointName != NAME_None;
}

void UPBBossDamageComponent::HandleHitPartComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	static_cast<void>(OtherActor);
	static_cast<void>(OtherComponent);
	static_cast<void>(NormalImpulse);
	static_cast<void>(Hit);

	if (const UPBBossHitPartComponent* HitPartComponent = FindHitPartComponent(HitComponent))
	{
		LastHitPointName = HitPartComponent->GetHitPointName();
		LastHitFrameNumber = GFrameCounter;
	}
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

void UPBBossDamageComponent::BindHitPartCollisionEvents()
{
	if (!OwnerBoss)
	{
		return;
	}

	TArray<UPBBossHitPartComponent*> HitPartComponents;
	OwnerBoss->GetComponents<UPBBossHitPartComponent>(HitPartComponents);

	for (const UPBBossHitPartComponent* HitPartComponent : HitPartComponents)
	{
		if (!HitPartComponent)
		{
			continue;
		}

		TArray<UPrimitiveComponent*> HitCollisionComponents;
		HitPartComponent->GetHitCollisionComponents(HitCollisionComponents);

		for (UPrimitiveComponent* HitCollisionComponent : HitCollisionComponents)
		{
			if (HitCollisionComponent)
			{
				HitCollisionComponent->SetNotifyRigidBodyCollision(true);
				HitCollisionComponent->OnComponentHit.AddUniqueDynamic(this, &UPBBossDamageComponent::HandleHitPartComponentHit);
			}
		}
	}
}

FName UPBBossDamageComponent::ResolveHitPointName() const
{
	if (LastHitFrameNumber == GFrameCounter && LastHitPointName != NAME_None)
	{
		return LastHitPointName;
	}

	return DefaultHitPointName;
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

bool UPBBossDamageComponent::IsWeakPointHitBlocked(FName HitPointName) const
{
	const UPBBossWeaknessComponent* WeaknessComponent = OwnerBoss ? OwnerBoss->GetBossWeaknessComponent() : nullptr;
	return WeaknessComponent
		&& WeaknessComponent->IsWeaknessPoint(HitPointName)
		&& !WeaknessComponent->IsWeaknessPointOpen(HitPointName);
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
