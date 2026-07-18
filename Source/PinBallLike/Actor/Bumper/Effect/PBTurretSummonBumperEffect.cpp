#include "PBTurretSummonBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PinBallLike/Actor/Bumper/Summon/PBTurretSummonActor.h"

void UPBTurretSummonBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	APBModularBumperBase* TargetBumper = IsValid(Bumper) ? Bumper : OwnerBumper.Get();
	const int32 ProjectileDamage = FMath::RoundToInt(EffectData.Power);
	if (!IsValid(TargetBumper)
		|| !IsValid(InteractionActor)
		|| ProjectileDamage <= 0
		|| !EnsureSummonActor(TargetBumper))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Turret effect skipped. Bumper=%s Target=%s Power=%.2f"),
			*GetNameSafe(TargetBumper),
			*GetNameSafe(InteractionActor),
			EffectData.Power);
		FinishEffect();
		return;
	}

	APBTurretSummonActor* TurretActor = Cast<APBTurretSummonActor>(SpawnedSummonActor);
	if (!IsValid(TurretActor))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Turret effect requires APBTurretSummonActor. Spawned=%s"),
			*GetNameSafe(SpawnedSummonActor));
		FinishEffect();
		return;
	}

	TurretActor->SetAttackPayload(
		EPBBumperProjectilePayload::BossDamage,
		ProjectileDamage);
	Super::ActivateEffectForActor(TargetBumper, InteractionActor);
}
