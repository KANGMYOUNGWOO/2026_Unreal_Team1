#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PBBounceStatusEffect.generated.h"

UCLASS()
class PINBALLLIKE_API UPBBounceStatusEffect : public UPBBaseStatusEffect
{
	GENERATED_BODY()

public:
	virtual void ExecuteStatusEffect(FName TriggerEvent) override;

private:
	int32 CalculateBouncinessBonus() const;
	void ApplyBouncinessDelta(int32 Delta) const;

	int32 AppliedBouncinessBonus = 0;
};
