#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBGameplayEffectParamRow.generated.h"

/** 공용 효과 하나에 연결되는 Key-Value 파라미터 한 행입니다. */
USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBGameplayEffectParamRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect|Parameter")
	FName EffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect|Parameter")
	FName ParamKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect|Parameter")
	FString ParamValue;
};
