#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PBEffectTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBEffectTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName EffectType = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName TargetType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName TargetFilter = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FGameplayTag TriggerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName Description = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName LinkedStatusEffectId = NAME_None;
};
