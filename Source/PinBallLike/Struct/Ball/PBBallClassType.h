#pragma once

#include "CoreMinimal.h"
#include "PBBallClassType.generated.h"

UENUM(BlueprintType)
enum class EPBBallClassType : uint8
{
	Attacker = 0 UMETA(DisplayName = "Attacker"),
	Debuffer = 1 UMETA(DisplayName = "Debuffer"),
	Supporter = 2 UMETA(DisplayName = "Supporter"),
	Healer = 3 UMETA(DisplayName = "Healer"),
	Tanker = 4 UMETA(DisplayName = "Tanker"),
	None = 255 UMETA(DisplayName = "None")
};
