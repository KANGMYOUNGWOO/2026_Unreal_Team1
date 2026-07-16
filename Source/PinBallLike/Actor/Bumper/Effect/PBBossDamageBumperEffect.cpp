#include "PBBossDamageBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Interface/BossInterface.h"

void UPBBossDamageBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static_cast<void>(InteractionActor);

	int32 DamageAmount = 0;
	AActor* BossTarget = FindBossTarget(Bumper);
	if (!IsValid(BossTarget) || !TryResolvePositivePower(DamageAmount))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Boss damage skipped. Bumper=%s Boss=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BossTarget),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const bool bApplied = IBossInterface::Execute_DamageToBoss(BossTarget, DamageAmount);
	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Boss damage requested. Bumper=%s Boss=%s Amount=%d Applied=%s"),
		*GetNameSafe(Bumper),
		*GetNameSafe(BossTarget),
		DamageAmount,
		bApplied ? TEXT("true") : TEXT("false"));

	FinishEffect();
}
