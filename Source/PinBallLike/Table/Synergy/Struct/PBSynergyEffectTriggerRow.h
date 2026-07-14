#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBSynergyEffectTriggerRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyEffectTriggerRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName SynergyEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName TargetEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	FName TargetTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy", meta = (ClampMin = "0", ClampMax = "100"))
	float ProcChance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	bool bScaleWithStack = false;
};
