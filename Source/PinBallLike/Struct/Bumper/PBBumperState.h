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
