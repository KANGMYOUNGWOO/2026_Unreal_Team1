#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBPercentShieldBumperEffect.generated.h"

/** Effect 시트의 Power를 최대 체력 대비 비율로 사용해 보호막을 즉시 충전합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBPercentShieldBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	/** 보호막의 누적 상한을 최대 체력의 몇 배로 둘지 결정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Shield",
		meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MaxShieldToHealthRatio = 1.0f;
};
