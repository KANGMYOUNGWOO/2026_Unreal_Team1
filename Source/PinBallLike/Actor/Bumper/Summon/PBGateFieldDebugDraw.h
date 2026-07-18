#pragma once

#include "CoreMinimal.h"

class UWorld;

/** 관문 영역의 실제 판정 중심과 반경을 PIE에서 확인하는 개발용 드로어입니다. */
namespace PBGateFieldDebugDraw
{
	PINBALLLIKE_API void Draw(
		const UWorld* World,
		const FVector& FieldCenter,
		float FieldRadius,
		const FColor& FieldColor,
		const TCHAR* FieldLabel,
		bool bHasTriggerOrigin,
		const FVector& TriggerOrigin);
}
