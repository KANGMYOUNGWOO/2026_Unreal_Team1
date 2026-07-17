#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBGameplayEffectRow.generated.h"

/**
 * 공용 효과 시트의 Effect 한 행입니다.
 * 문자열 ID를 그대로 보존해 다른 시스템이 추가한 효과도 테이블에 안전하게 적재합니다.
 * 실제 범퍼 실행 가능 여부는 UPBBumperSharedEffectAdapter가 별도로 검증합니다.
 */
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
