#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTypes.h"
#include "PBStatusEffectModifierRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBStatusEffectModifierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName StatusEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EPBStatusEffectModifyType ModifyType = EPBStatusEffectModifyType::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	bool bScaleWithStack = false;
};
