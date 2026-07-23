#pragma once

#include "CoreMinimal.h"
#include "PBBallSkillActivatedMessage.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallSkillActivatedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Ball")
	TObjectPtr<AActor> SkillOwnerBall = nullptr;
};
