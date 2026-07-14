#pragma once

#include "CoreMinimal.h"
#include "PBBumperState.generated.h"

UENUM(BlueprintType)
enum class EPBBumperState : uint8
{
	Idle,
	Activated,
	Disabled
};

/** 물리 Trigger 인스턴스 하나의 독립적인 충전·발동 진행 상태다. */
UENUM(BlueprintType)
enum class EPBBumperTriggerProgressState : uint8
{
	Charging,
	Ready,
	Queued,
	Executing
};
