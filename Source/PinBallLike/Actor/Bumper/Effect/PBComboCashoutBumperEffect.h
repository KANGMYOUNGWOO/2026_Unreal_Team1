#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBComboCashoutBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBComboCashoutBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Combo Cashout",
		meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxComboConsumption = 20;
};
