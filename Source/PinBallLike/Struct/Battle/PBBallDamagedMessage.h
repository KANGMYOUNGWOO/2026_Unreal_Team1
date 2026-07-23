#pragma once

#include "CoreMinimal.h"
#include "PBBallDamagedMessage.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDamagedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Ball")
	int32 AppliedDamage = 0;
};
