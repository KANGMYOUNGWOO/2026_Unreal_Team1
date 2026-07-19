#pragma once

#include "CoreMinimal.h"
#include "PBSynergyState.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|State")
	FName SynergyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|State")
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|State")
	int32 ActiveRequiredCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|State")
	FName ActiveEffectSetId = NAME_None;

	bool IsValid() const
	{
		return !SynergyId.IsNone() && CurrentCount > 0;
	}

	bool operator==(const FPBSynergyState& Other) const
	{
		return SynergyId == Other.SynergyId
			&& CurrentCount == Other.CurrentCount
			&& ActiveRequiredCount == Other.ActiveRequiredCount
			&& ActiveEffectSetId == Other.ActiveEffectSetId;
	}
};
