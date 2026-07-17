#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBCounterShieldBumperEffect.generated.h"

/** 고정 보호막을 충전하고 첫 보호막 피해에 보스 추적 반격탄을 예약합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBCounterShieldBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	/** 보호막 최대치가 아직 없을 때 최대 체력 대비 이 비율로 보호막 저장 공간을 만듭니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Counter Shield",
		meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MaxShieldToHealthRatio = 1.0f;
};
