#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBComboCashoutBumperEffect.generated.h"

/**
 * 충돌한 Ball의 현재 콤보를 모두 소비하고 콤보 x Effect Power만큼 보스 피해를 줍니다.
 * 보스 피해 적용이 실패하면 콤보는 소비하지 않습니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBComboCashoutBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
