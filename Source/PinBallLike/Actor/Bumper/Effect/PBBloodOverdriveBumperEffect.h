#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBloodOverdriveBumperEffect.generated.h"

/** HP를 최소 잔여치 아래로 내리지 않는 경우에만 소비하고, 같은 Ball에 임시 공격력 보정을 적용합니다. */
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
