#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBSynergyRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName DescriptionKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	int32 SortOrder = 0;
};
