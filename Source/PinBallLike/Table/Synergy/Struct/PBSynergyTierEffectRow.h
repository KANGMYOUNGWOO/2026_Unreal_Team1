#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBSynergyTierEffectRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyTierEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName SynergyTierId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName SynergyEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	int32 ApplyOrder = 0;
};
