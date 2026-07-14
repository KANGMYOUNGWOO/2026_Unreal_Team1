#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBSynergyTierRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyTierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName SynergyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	int32 RequiredCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName TierDescriptionKey = NAME_None;
};
