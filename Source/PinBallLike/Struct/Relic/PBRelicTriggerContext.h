#pragma once

#include "CoreMinimal.h"
#include "PBRelicTriggerContext.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBRelicTriggerContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Relic")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Relic")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Relic")
	int32 IntValue = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Relic")
	float FloatValue = 0.f;
};