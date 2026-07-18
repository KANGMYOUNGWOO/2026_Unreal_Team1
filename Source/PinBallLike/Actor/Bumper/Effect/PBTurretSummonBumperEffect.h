#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PBTurretSummonBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBTurretSummonBumperEffect : public UPBSummonBumperEffect
{
	GENERATED_BODY()

public:
	UPBTurretSummonBumperEffect();

	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
