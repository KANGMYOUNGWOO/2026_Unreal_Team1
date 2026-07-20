#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBCounterShieldBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBCounterShieldBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Counter Shield",
		meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MaxShieldToHealthRatio = 1.0f;
};
