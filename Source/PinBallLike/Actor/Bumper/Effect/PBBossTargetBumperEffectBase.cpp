#include "PBBossTargetBumperEffectBase.h"

#include "EngineUtils.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossSpawner.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"

namespace
{
	constexpr float MaxBossEffectPower = 1000000.0f;
}

AActor* UPBBossTargetBumperEffectBase::FindBossTarget(const UObject* WorldContext) const
{
	UWorld* World = IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return nullptr;
	}

	for (TActorIterator<APBBossSpawner> It(World); It; ++It)
	{
		if (APBBossSpawner* BossSpawner = *It)
		{
			if (APBBossBase* SpawnedBoss = BossSpawner->GetSpawnedBoss())
			{
				return SpawnedBoss;
			}
		}
	}

	for (TActorIterator<APBBossBase> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			return *It;
		}
	}

	return nullptr;
}

bool UPBBossTargetBumperEffectBase::TryResolvePositivePower(int32& OutPower) const
{
	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		OutPower = 0;
		return false;
	}

	OutPower = FMath::Max(
		FMath::RoundToInt(FMath::Min(EffectData.Power, MaxBossEffectPower)),
		1);
	return true;
}

bool UPBBossTargetBumperEffectBase::SpawnBossProjectile(
	APBModularBumperBase* Bumper,
	AActor* BossTarget,
	const EPBBumperProjectilePayload Payload,
	const int32 Power,
	const float PayloadDuration) const
{
	if (!IsValid(Bumper) || !IsValid(BossTarget) || !ProjectileClass || Power <= 0)
	{
		return false;
	}

	UWorld* World = Bumper->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	const FTransform SourceTransform = IsValid(ActiveTrigger)
		? ActiveTrigger->GetActorTransform()
		: Bumper->GetActorTransform();
	const FVector SpawnLocation = SourceTransform.TransformPosition(ProjectileSpawnOffset);
	const FVector TargetDirection = BossTarget->GetActorLocation() - SpawnLocation;
	const FRotator SpawnRotation = TargetDirection.IsNearlyZero()
		? SourceTransform.Rotator()
		: TargetDirection.Rotation();

	APBBumperProjectile* Projectile = APBBumperProjectile::SpawnForTarget(
		Bumper,
		ProjectileClass,
		Bumper,
		SpawnLocation,
		SpawnRotation,
		BossTarget,
		Payload,
		Power,
		PayloadDuration,
		ProjectileLifetime,
		Bumper->GetDeliveryVfx(),
		Bumper->GetImpactVfx(),
		Bumper->GetStatusVfx());
	return IsValid(Projectile);
}
