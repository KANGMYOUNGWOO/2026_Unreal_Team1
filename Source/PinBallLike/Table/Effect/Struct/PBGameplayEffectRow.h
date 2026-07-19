#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBGameplayEffectRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBGameplayEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FName EffectType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FName TargetType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FName TargetFilter = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FName TriggerTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FName LinkedStatusEffectId = NAME_None;
};
