#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBloodOverdriveBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBloodOverdriveBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Blood Overdrive")
	FName EffectSourceId = TEXT("BumperBloodOverdrive");
};
