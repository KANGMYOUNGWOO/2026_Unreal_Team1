#pragma once

#include "CoreMinimal.h"
#include "PBBumperTriggeredMessage.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperTriggeredMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Bumper")
	TObjectPtr<AActor> Bumper;

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Bumper")
	FVector TriggerLocation = FVector::ZeroVector;
};
