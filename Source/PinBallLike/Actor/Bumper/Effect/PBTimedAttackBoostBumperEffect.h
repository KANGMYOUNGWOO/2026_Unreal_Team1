#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBTimedAttackBoostBumperEffect.generated.h"

/** Effect 시트의 Power를 공격력 증가율로 사용해 충돌한 Ball에 한시적 보정을 적용합니다. */
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
