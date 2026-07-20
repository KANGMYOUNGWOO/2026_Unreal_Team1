#include "PinBallLike/Actor/StatusEffect/Effects/PBPiercingStatusEffect.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"

void UPBPiercingStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
	if (TriggerEvent == PBStatusEffectTriggerEvents::Applied)
	{
		if (APBBallBase* OwnerBall = GetOwnerBall())
		{
			OwnerBall->AddBossCollisionIgnoreRequest(this);
			bBossCollisionIgnoreRequested = true;
		}
		return;
	}

	if (TriggerEvent == PBStatusEffectTriggerEvents::Removed
		|| TriggerEvent == PBStatusEffectTriggerEvents::Expired)
	{
		DeactivatePiercing();
	}
}

void UPBPiercingStatusEffect::DeactivatePiercing()
{
	if (bBossCollisionIgnoreRequested)
	{
		if (APBBallBase* OwnerBall = GetOwnerBall())
		{
			OwnerBall->RemoveBossCollisionIgnoreRequest(this);
		}
	}

	bBossCollisionIgnoreRequested = false;
}

APBBallBase* UPBPiercingStatusEffect::GetOwnerBall() const
{
	return Cast<APBBallBase>(GetOwnerActor());
}
