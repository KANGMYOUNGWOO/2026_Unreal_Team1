#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PBZeroGravityStatusEffect.generated.h"

UCLASS()
class PINBALLLIKE_API UPBZeroGravityStatusEffect : public UPBBaseStatusEffect
{
	GENERATED_BODY()

public:
	virtual void ExecuteStatusEffect(FName TriggerEvent) override;
};
