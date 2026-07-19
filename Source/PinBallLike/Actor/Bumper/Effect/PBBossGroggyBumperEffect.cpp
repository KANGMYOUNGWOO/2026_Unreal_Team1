#include "PBBossGroggyBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"

void UPBBossGroggyBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static_cast<void>(InteractionActor);

	int32 GroggyAmount = 0;
	AActor* BossTarget = FindBossTarget(Bumper);
	if (!IsValid(BossTarget) || !TryResolvePositivePower(GroggyAmount))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Boss groggy skipped. Bumper=%s Boss=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BossTarget),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const bool bSpawned = SpawnBossProjectile(
		Bumper,
		BossTarget,
		EPBBumperProjectilePayload::BossGroggy,
		GroggyAmount);
	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Boss groggy projectile requested. Bumper=%s Boss=%s Amount=%d Spawned=%s"),
		*GetNameSafe(Bumper),
		*GetNameSafe(BossTarget),
		GroggyAmount,
		bSpawned ? TEXT("true") : TEXT("false"));

	FinishEffect();
}
