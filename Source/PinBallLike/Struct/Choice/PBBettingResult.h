#pragma once

#include "CoreMinimal.h"
#include "PBBettingResult.generated.h"

USTRUCT(BlueprintType)
struct FPBBettingResult
{
	GENERATED_BODY()

	// 플레이어가 선택한 국가
	UPROPERTY(BlueprintReadOnly)
	int32 SelectedIndex = INDEX_NONE;

	// 실제 승리 국가
	UPROPERTY(BlueprintReadOnly)
	int32 WinnerIndex = INDEX_NONE;

	// 내기 성공 여부
	UPROPERTY(BlueprintReadOnly)
	bool bWin = false;
};