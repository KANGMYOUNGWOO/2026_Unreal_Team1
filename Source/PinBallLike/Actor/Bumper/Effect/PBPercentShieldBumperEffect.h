#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBPercentShieldBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBPercentShieldBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Shield",
		meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MaxShieldToHealthRatio = 1.0f;
};
