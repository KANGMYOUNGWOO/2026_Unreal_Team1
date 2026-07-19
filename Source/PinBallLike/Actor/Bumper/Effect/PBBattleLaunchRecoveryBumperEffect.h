#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBattleLaunchRecoveryBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBattleLaunchRecoveryBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Battle Launch",
		meta = (ClampMin = "1", ClampMax = "99"))
	int32 MaxRemainingLaunchCount = 9;
};
