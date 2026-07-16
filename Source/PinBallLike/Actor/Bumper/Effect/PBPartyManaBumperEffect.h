#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBPartyManaBumperEffect.generated.h"

/** 범퍼 발동 시 현재 생존 파티원들의 마나를 Effect 시트의 Power만큼 회복합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBPartyManaBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
