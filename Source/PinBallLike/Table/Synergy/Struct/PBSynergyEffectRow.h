#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTypes.h"
#include "PBSynergyEffectRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	EPBStatusEffectStackType StackType = EPBStatusEffectStackType::Replace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	EPBStatusEffectDurationPolicy DurationPolicy = EPBStatusEffectDurationPolicy::Permanent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy", meta = (ClampMin = "0"))
	float DurationValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy", meta = (ClampMin = "0"))
	float Interval = 0.0f;
};
