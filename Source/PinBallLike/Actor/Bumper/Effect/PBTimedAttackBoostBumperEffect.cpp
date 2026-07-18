#include "PBTimedAttackBoostBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Reward/PBBumperRewardUtils.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"

void UPBTimedAttackBoostBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	const float ResolvedDuration = EffectData.Duration > 0.0f
		? EffectData.Duration
		: EffectDuration;
	const FPBBumperRewardApplyResult Result = PBBumperRewardUtils::ApplyReward(
		InteractionActor,
		EPBBumperRewardType::TimedAttackPercent,
		NAME_None,
		NAME_None,
		EffectData.Power,
		EffectSourceId,
		ResolvedDuration);
	if (Result.bApplied)
	{
		PlayResolvedVfx(InteractionActor, ResolvedDuration, true);
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Timed attack boost resolved. Bumper=%s Target=%s Applied=%s Percent=%.1f Bonus=%d Duration=%.1f"),
		*GetNameSafe(Bumper),
		*GetNameSafe(InteractionActor),
		Result.bApplied ? TEXT("true") : TEXT("false"),
		EffectData.Power,
		FMath::RoundToInt(Result.AppliedValue),
		ResolvedDuration);

	FinishEffect();
}
