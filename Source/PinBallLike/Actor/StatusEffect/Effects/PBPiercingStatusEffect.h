#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PBPiercingStatusEffect.generated.h"

class APBBallBase;

UCLASS()
class PINBALLLIKE_API UPBPiercingStatusEffect : public UPBBaseStatusEffect
{
	GENERATED_BODY()

public:
	virtual void ExecuteStatusEffect(FName TriggerEvent) override;

private:
	void DeactivatePiercing();
	APBBallBase* GetOwnerBall() const;

	bool bBossCollisionIgnoreRequested = false;
};
