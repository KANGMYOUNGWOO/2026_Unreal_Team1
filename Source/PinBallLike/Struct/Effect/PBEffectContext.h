#pragma once

#include "CoreMinimal.h"
#include "PBEffectContext.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBEffectContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TArray<TObjectPtr<AActor>> TargetActors;
};
