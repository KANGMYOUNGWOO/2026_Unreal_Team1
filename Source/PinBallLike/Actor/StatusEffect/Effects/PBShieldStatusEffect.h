#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PBShieldStatusEffect.generated.h"

UCLASS()
class PINBALLLIKE_API UPBShieldStatusEffect : public UPBBaseStatusEffect
{
	GENERATED_BODY()

public:
	virtual void ExecuteStatusEffect(FName TriggerEvent) override;

private:
	float CalculateShieldValue() const;
	float ApplyShieldDelta(float Delta) const;

	float AppliedShieldValue = 0.0f;
};
