#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTypes.h"
#include "PBStatusEffectRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBStatusEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName StatusEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName DisplayNameKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FName DescriptionKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EPBStatusEffectKind StatusEffectKind = EPBStatusEffectKind::Marker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FString Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EPBStatusEffectStackType StackType = EPBStatusEffectStackType::Replace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EPBStatusEffectDurationPolicy DurationPolicy = EPBStatusEffectDurationPolicy::Permanent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", meta = (ClampMin = "0"))
	float DurationValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", meta = (ClampMin = "0"))
	float Interval = 0.0f;
};
