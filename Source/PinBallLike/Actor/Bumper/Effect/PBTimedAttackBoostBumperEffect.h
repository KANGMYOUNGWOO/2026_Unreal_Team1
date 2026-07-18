#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBTimedAttackBoostBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBTimedAttackBoostBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Attack Boost")
	FName EffectSourceId = TEXT("BumperStrengthCharge");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Attack Boost",
		meta = (ClampMin = "0.1", ClampMax = "60.0", Units = "s"))
	float EffectDuration = 5.0f;
};
