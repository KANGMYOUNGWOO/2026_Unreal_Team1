#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTypes.h"
#include "PBSynergyEffectModifierRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyEffectModifierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName SynergyEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	EPBStatusEffectModifyType ModifyType = EPBStatusEffectModifyType::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	bool bScaleWithStack = false;
};
