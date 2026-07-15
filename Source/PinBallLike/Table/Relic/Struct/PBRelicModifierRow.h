#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Relic/PBRelicTypes.h"
#include "PBRelicModifierRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBRelicModifierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName RelicId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	EPBRelicTargetType TargetType = EPBRelicTargetType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName TargetStat = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	EPBRelicModifyType ModifyType = EPBRelicModifyType::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	float Value = 0.0f;
};