#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Relic/PBRelicTypes.h"
#include "PBRelicTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBRelicTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	EPBRelicRarity Rarity = EPBRelicRarity::Common;
	
};