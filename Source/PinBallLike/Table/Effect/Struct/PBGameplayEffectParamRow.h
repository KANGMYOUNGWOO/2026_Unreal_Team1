#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBGameplayEffectParamRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBGameplayEffectParamRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect|Parameter")
	FName EffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect|Parameter")
	FName ParamKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect|Parameter")
	FString ParamValue;
};
