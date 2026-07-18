#include "PBBumperComboArcComponent.h"

#include "PinBallLike/Actor/Ball/Component/PBBallComboComponent.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "TimerManager.h"

namespace
{
	constexpr float MaxComboArcDuration = 60.0f;
	constexpr int32 MaxComboArcInterval = 100000;
	constexpr int32 MaxComboArcDamage = 1000000;
	constexpr int32 MaxArcShotsPerComboEvent = 100;
}

UPBBumperComboArcComponent::UPBBumperComboArcComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBumperComboArcComponent::Arm(
	APBModularBumperBase* SourceBumper,
	AActor* BossTarget,
	TSubclassOf<APBBumperProjectile> ProjectileClass,
	const int32 ComboInterval,
	const int32 Damage,
	const float Duration,
	const FVector& SpawnOffset,
	const float ProjectileLifetime)
{
	UPBBallComboComponent* ComboComponent = ResolveComboComponent();
	UWorld* World = GetWorld();
	if (!IsValid(ComboComponent)
		|| !IsValid(World)
		|| !IsValid(SourceBumper)
		|| !IsValid(BossTarget)
		|| !ProjectileClass
		|| ComboInterval <= 0
		|| Damage <= 0
		|| !FMath::IsFinite(Duration)
		|| Duration <= 0.0f)
	{
		return false;
	}

	if (!bDelegateBound)
	{
		ComboComponent->OnComboChanged.AddDynamic(
			this,
			&UPBBumperComboArcComponent::HandleComboChanged);
		bDelegateBound = true;
	}

	World->GetTimerManager().ClearTimer(ExpirationTimer);
	if (!bArmed)
	{
		AccumulatedComboGain = 0;
	}
	ArmedBumper = SourceBumper;
	ArmedBossTarget = BossTarget;
	ArmedProjectileClass = ProjectileClass;
	ArmedSpawnOffset = SpawnOffset;
	ArmedComboInterval = FMath::Clamp(ComboInterval, 1, MaxComboArcInterval);
	ArmedDamage = FMath::Clamp(Damage, 1, MaxComboArcDamage);
	ArmedProjectileLifetime = FMath::Max(ProjectileLifetime, 0.1f);
	LastCombo = ComboComponent->GetCombo();
	bArmed = true;
	World->GetTimerManager().SetTimer(
		ExpirationTimer,
		this,
		&UPBBumperComboArcComponent::Disarm,
		FMath::Clamp(Duration, 0.1f, MaxComboArcDuration),
		false);
	return true;
}

void UPBBumperComboArcComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Disarm();
	if (UPBBallComboComponent* ComboComponent = ResolveComboComponent())
	{
		ComboComponent->OnComboChanged.RemoveDynamic(
			this,
			&UPBBumperComboArcComponent::HandleComboChanged);
	}
	bDelegateBound = false;
	Super::EndPlay(EndPlayReason);
}

UPBBallComboComponent* UPBBumperComboArcComponent::ResolveComboComponent()
{
	if (CachedComboComponent.IsValid())
	{
		return CachedComboComponent.Get();
	}

	AActor* OwnerActor = GetOwner();
	UPBBallComboComponent* ComboComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBBallComboComponent>()
		: nullptr;
	CachedComboComponent = ComboComponent;
	return ComboComponent;
}

void UPBBumperComboArcComponent::Disarm()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationTimer);
	}
	bArmed = false;
	ArmedBumper.Reset();
	ArmedBossTarget.Reset();
	ArmedProjectileClass = nullptr;
	ArmedComboInterval = 0;
	ArmedDamage = 0;
	AccumulatedComboGain = 0;
}

bool UPBBumperComboArcComponent::FireArcProjectile() const
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

void UPBBumperComboArcComponent::HandleComboChanged(
	const int32 CurrentCombo,
	const int32 MaxCombo)
{
	static_cast<void>(MaxCombo);
	const int32 ComboGain = CurrentCombo - LastCombo;
	LastCombo = CurrentCombo;
	if (!bArmed || ComboGain <= 0 || ArmedComboInterval <= 0)
	{
		return;
	}

	AccumulatedComboGain = FMath::Max(AccumulatedComboGain + ComboGain, 0);
	const int32 AvailableShots = FMath::Min(
		AccumulatedComboGain / ArmedComboInterval,
		MaxArcShotsPerComboEvent);
	int32 SpawnedShots = 0;
	for (int32 ShotIndex = 0; ShotIndex < AvailableShots; ++ShotIndex)
	{
		if (!FireArcProjectile())
		{
			break;
		}
		AccumulatedComboGain -= ArmedComboInterval;
		++SpawnedShots;
	}

	if (SpawnedShots > 0)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Combo arc fired. Target=%s Gain=%d Shots=%d Remainder=%d"),
			*GetNameSafe(GetOwner()),
			ComboGain,
			SpawnedShots,
			AccumulatedComboGain);
	}
	else
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("[Bumper] Combo arc progress. Target=%s Gain=%d Remainder=%d"),
			*GetNameSafe(GetOwner()),
			ComboGain,
			AccumulatedComboGain);
	}
}
