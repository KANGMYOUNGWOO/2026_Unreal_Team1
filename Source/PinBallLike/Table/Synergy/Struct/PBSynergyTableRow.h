#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Synergy/PBSynergyTypes.h"
#include "PBSynergyTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName DescriptionKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	EPBSynergyKind SynergyKind = EPBSynergyKind::Race;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	EPBSynergyRuleType RuleType = EPBSynergyRuleType::Stat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	int32 SortOrder = 0;
};
