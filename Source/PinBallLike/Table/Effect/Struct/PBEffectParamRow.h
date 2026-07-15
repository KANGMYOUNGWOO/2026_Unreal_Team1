#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBEffectParamRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBEffectParamRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName EffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName ParamKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FString ParamValue;
};
