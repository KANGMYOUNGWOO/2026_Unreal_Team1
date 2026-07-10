#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTypes.h"
#include "PBStatusEffectTriggerRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBStatusEffectTriggerRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName StatusEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	bool bScaleWithStack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName TargetStatusEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName TargetTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", meta = (ClampMin = "0", ClampMax = "100"))
	float ProcChance = 100.0f;
};
