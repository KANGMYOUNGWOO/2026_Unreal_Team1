#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBBossGroggyBumperEffect.generated.h"

/** 범퍼 발동 시 현재 보스의 그로기 게이지를 Effect 시트의 Power만큼 증가시킵니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossGroggyBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
