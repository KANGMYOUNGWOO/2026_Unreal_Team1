#pragma once

#include "CoreMinimal.h"
#include "PBChoiceNodeTypes.generated.h"



UENUM(BlueprintType)
enum class EPBChoiceNodeType : uint8
{
	   None    UMETA(DisplayName = "None"),
	   Shop    UMETA(DisplayName = "Shop"),
	   Enhance UMETA(DisplayName = "Enhance"),
	   Bet     UMETA(DisplayName = "Bet"),
	   Battle  UMETA(DisplayName = "Battle"),
	   Event   UMETA(DisplayName = "Event"),
	   Boss    UMETA(DisplayName = "Boss")
};


UENUM(BlueprintType)
enum class EPBChoiceRouteSide : uint8
{
	Left    UMETA(DisplayName = "Left"),
	Right   UMETA(DisplayName = "Right")
};