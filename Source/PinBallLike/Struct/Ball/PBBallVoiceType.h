#pragma once

#include "CoreMinimal.h"
#include "PBBallVoiceType.generated.h"

UENUM(BlueprintType)
enum class EPBBallVoiceType : uint8
{
	Girl UMETA(DisplayName = "Girl"),
	Boy UMETA(DisplayName = "Boy")
};
