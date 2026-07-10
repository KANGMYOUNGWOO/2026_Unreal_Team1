#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PBBurnStatusEffect.generated.h"

UCLASS()
class PINBALLLIKE_API UPBBurnStatusEffect : public UPBBaseStatusEffect
{
	GENERATED_BODY()

public:
	virtual void ExecuteStatusEffect(FName TriggerEvent) override;

private:
	int32 CalculateDamage() const;
};
