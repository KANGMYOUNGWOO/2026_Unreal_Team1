#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBBossGroggyBumperEffect.generated.h"

/** 범퍼 발동 시 보스를 추적하는 탄환을 발사하고, 명중 시 Effect 시트의 Power만큼 그로기를 줍니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossGroggyBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
