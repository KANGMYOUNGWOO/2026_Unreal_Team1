#pragma once

#include "CoreMinimal.h"

class UWorld;

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
