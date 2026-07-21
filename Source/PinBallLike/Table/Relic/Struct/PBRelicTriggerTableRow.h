#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PBRelicTriggerTableRow.generated.h"

UENUM(BlueprintType)
enum class ERelicTriggerCondition : uint8
{
	None,

	GreaterOrEqual,

	LessOrEqual,

	Equal
};

USTRUCT(BlueprintType)
struct FPBRelicTriggerTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName RelicId;

	UPROPERTY(EditAnywhere)
	FGameplayTag TriggerTag;

	UPROPERTY(EditAnywhere)
	ERelicTriggerCondition ConditionType;

	UPROPERTY(EditAnywhere)
	int32 ConditionValue;

	UPROPERTY(EditAnywhere)
	FName StatusEffectId;
};