#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternTelegraph.h"
#include "PBBossChargeTelegraph.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PINBALLLIKE_API APBBossChargeTelegraph : public APBBossPatternTelegraph
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	void InitChargeTelegraph(float InDurationSeconds, const FVector& StartLocation, const FVector& Direction, float Length);
};
