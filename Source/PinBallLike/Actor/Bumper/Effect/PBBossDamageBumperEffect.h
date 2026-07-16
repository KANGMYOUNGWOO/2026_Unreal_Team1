#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBBossDamageBumperEffect.generated.h"

/** 범퍼 발동 시 현재 보스에게 Effect 시트의 Power만큼 직접 피해를 요청합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossDamageBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
