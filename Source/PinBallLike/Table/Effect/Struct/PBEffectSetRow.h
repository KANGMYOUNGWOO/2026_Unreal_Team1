#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBEffectSetRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBEffectSetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName EffectSetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName EffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	int32 Order = 0;
};
