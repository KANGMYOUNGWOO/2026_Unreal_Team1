#pragma once

#include "CoreMinimal.h"
#include "PBBallClassType.generated.h"

UENUM(BlueprintType)
enum class EPBBallClassType : uint8
{
	Attacker UMETA(DisplayName = "Attacker"),
	Debuffer UMETA(DisplayName = "Debuffer"),
	Supporter UMETA(DisplayName = "Supporter"),
	Healer UMETA(DisplayName = "Healer"),
	Tanker UMETA(DisplayName = "Tanker")
};
