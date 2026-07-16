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

	// 에디터 단독 테스트처럼 Spawner 없이 배치된 보스 본체도 지원한다.
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
	const int32 Power) const
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

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Bumper;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APBBumperProjectile* Projectile = World->SpawnActor<APBBumperProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!IsValid(Projectile))
	{
		return false;
	}

	Projectile->ConfigureForTarget(BossTarget, Payload, Power, true);
	Projectile->SetLifeSpan(FMath::Max(ProjectileLifetime, 0.1f));
	Projectile->ActivateProjectile();
	return true;
}
