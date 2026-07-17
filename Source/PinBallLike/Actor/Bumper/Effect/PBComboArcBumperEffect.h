#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBComboArcBumperEffect.generated.h"

/** 일정 시간 동안 증가한 콤보를 공용 효과 시트 구간마다 보스 추적탄으로 전환합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBComboArcBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
