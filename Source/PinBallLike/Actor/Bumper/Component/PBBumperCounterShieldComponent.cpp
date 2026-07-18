#include "PBBumperCounterShieldComponent.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "TimerManager.h"

namespace
{
	constexpr float MaxCounterShieldDuration = 60.0f;
	constexpr int32 MaxCounterShieldDamage = 1000000;
}

UPBBumperCounterShieldComponent::UPBBumperCounterShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBumperCounterShieldComponent::Arm(
	APBModularBumperBase* SourceBumper,
	AActor* BossTarget,
	TSubclassOf<APBBumperProjectile> ProjectileClass,
	const int32 Damage,
	const float Duration,
	const FVector& SpawnOffset,
	const float ProjectileLifetime)
{
	UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent();
	UWorld* World = GetWorld();
	if (!IsValid(ResourceComponent)
		|| !IsValid(World)
		|| !ResourceComponent->HasResource(PBResourceNames::Shield)
		|| !IsValid(SourceBumper)
		|| !IsValid(BossTarget)
		|| !ProjectileClass
		|| Damage <= 0
		|| !FMath::IsFinite(Duration)
		|| Duration <= 0.0f)
	{
		return false;
	}

	if (!bDelegatesBound)
	{
		ResourceComponent->OnResourceChanged.AddUObject(
			this,
			&UPBBumperCounterShieldComponent::HandleResourceStructureChanged);
		ResourceComponent->OnResourceCurrentChanged.AddUObject(
			this,
			&UPBBumperCounterShieldComponent::HandleResourceCurrentChanged);
		bDelegatesBound = true;
	}

	World->GetTimerManager().ClearTimer(ExpirationTimer);
	ArmedBumper = SourceBumper;
	ArmedBossTarget = BossTarget;
	ArmedProjectileClass = ProjectileClass;
	ArmedSpawnOffset = SpawnOffset;
	ArmedDamage = FMath::Clamp(Damage, 1, MaxCounterShieldDamage);
	ArmedProjectileLifetime = FMath::Max(ProjectileLifetime, 0.1f);
	LastShieldValue = ResourceComponent->GetResourceCurrent(PBResourceNames::Shield);
	bArmed = true;
	World->GetTimerManager().SetTimer(
		ExpirationTimer,
		this,
		&UPBBumperCounterShieldComponent::Disarm,
		FMath::Clamp(Duration, 0.1f, MaxCounterShieldDuration),
		false);
	return true;
}

void UPBBumperCounterShieldComponent::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	Disarm();
	if (UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent())
	{
		ResourceComponent->OnResourceChanged.RemoveAll(this);
		ResourceComponent->OnResourceCurrentChanged.RemoveAll(this);
	}
	bDelegatesBound = false;
	Super::EndPlay(EndPlayReason);
}

UPBBaseResourceComponent* UPBBumperCounterShieldComponent::ResolveResourceComponent()
{
	if (CachedResourceComponent.IsValid())
	{
		return CachedResourceComponent.Get();
	}

	AActor* OwnerActor = GetOwner();
	UPBBaseResourceComponent* ResourceComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBBaseResourceComponent>()
		: nullptr;
	CachedResourceComponent = ResourceComponent;
	return ResourceComponent;
}

void UPBBumperCounterShieldComponent::Disarm()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationTimer);
	}
	bArmed = false;
	ArmedBumper.Reset();
	ArmedBossTarget.Reset();
	ArmedProjectileClass = nullptr;
	ArmedDamage = 0;
}

bool UPBBumperCounterShieldComponent::FireCounterProjectile() const
{
	APBModularBumperBase* Bumper = ArmedBumper.Get();
	AActor* BossTarget = ArmedBossTarget.Get();
	if (!IsValid(Bumper) || !IsValid(BossTarget) || !ArmedProjectileClass)
	{
		return false;
	}

	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	const FTransform SourceTransform = IsValid(ActiveTrigger)
		? ActiveTrigger->GetActorTransform()
		: Bumper->GetActorTransform();
	const FVector SpawnLocation = SourceTransform.TransformPosition(ArmedSpawnOffset);
	const FVector TargetDirection = BossTarget->GetActorLocation() - SpawnLocation;
	const FRotator SpawnRotation = TargetDirection.IsNearlyZero()
		? SourceTransform.Rotator()
		: TargetDirection.Rotation();
	return IsValid(APBBumperProjectile::SpawnForTarget(
		Bumper,
		ArmedProjectileClass,
		Bumper,
		SpawnLocation,
		SpawnRotation,
		BossTarget,
		EPBBumperProjectilePayload::BossDamage,
		ArmedDamage,
		0.0f,
		ArmedProjectileLifetime,
		Bumper->GetDeliveryVfx(),
		Bumper->GetImpactVfx(),
		nullptr));
}

void UPBBumperCounterShieldComponent::HandleResourceStructureChanged(
	const FName ChangedResourceName)
{
	if (ChangedResourceName == PBResourceNames::Shield)
	{
		if (const UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent())
		{
			LastShieldValue = ResourceComponent->GetResourceCurrent(PBResourceNames::Shield);
		}
	}
}

void UPBBumperCounterShieldComponent::HandleResourceCurrentChanged(
	const FName ChangedResourceName,
	const float NewValue)
{
	if (ChangedResourceName != PBResourceNames::Shield)
	{
		return;
	}

	const float PreviousValue = LastShieldValue;
	LastShieldValue = NewValue;
	if (!bArmed || NewValue >= PreviousValue - KINDA_SMALL_NUMBER)
	{
		return;
	}

	const bool bProjectileSpawned = FireCounterProjectile();
	if (bProjectileSpawned)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Counter shield consumed. Target=%s ShieldBefore=%.2f ShieldAfter=%.2f Spawned=true"),
			*GetNameSafe(GetOwner()),
			PreviousValue,
			NewValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Counter shield was consumed but its projectile failed. Target=%s"),
			*GetNameSafe(GetOwner()));
	}
	Disarm();
}
