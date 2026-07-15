#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PBStrengthStatusEffect.generated.h"

UCLASS()
class PINBALLLIKE_API UPBStrengthStatusEffect : public UPBBaseStatusEffect
{
	GENERATED_BODY()

public:
	virtual void ExecuteStatusEffect(FName TriggerEvent) override;

private:
	int32 CalculateAttackBonus() const;
	void ApplyAttackDelta(int32 Delta) const;

	int32 AppliedAttackBonus = 0;
};
