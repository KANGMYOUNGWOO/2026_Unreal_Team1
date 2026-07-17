#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBKineticShellBumperEffect.generated.h"

/** 충돌 Ball의 현재 수평 속도를 피해로 환산해 보스 추적 포탄을 발사합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBKineticShellBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
