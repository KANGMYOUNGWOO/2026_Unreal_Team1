#include "PBDirectRewardBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Reward/PBBumperRewardUtils.h"

void UPBDirectRewardBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	const FPBBumperRewardApplyResult Result = PBBumperRewardUtils::ApplyReward(
		InteractionActor,
		RewardType,
		ResourceName,
		StatusEffectId,
		EffectData.Power);

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Direct reward resolved. Bumper=%s Target=%s Applied=%s Value=%.2f Count=%d"),
		*GetNameSafe(Bumper),
		*GetNameSafe(InteractionActor),
		Result.bApplied ? TEXT("true") : TEXT("false"),
		Result.AppliedValue,
		Result.AppliedCount);

	FinishEffect();
}
