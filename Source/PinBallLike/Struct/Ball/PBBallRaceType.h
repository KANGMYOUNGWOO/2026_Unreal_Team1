#pragma once

#include "CoreMinimal.h"
#include "PBBallRaceType.generated.h"

UENUM(BlueprintType)
enum class EPBBallRaceType : uint8
{
	Dragon UMETA(DisplayName = "Dragon"),
	Human UMETA(DisplayName = "Human"),
	Spirit UMETA(DisplayName = "Spirit"),
	Beast UMETA(DisplayName = "Beast"),
	Yokai UMETA(DisplayName = "Yokai"),
	Machine UMETA(DisplayName = "Machine"),
	Plant UMETA(DisplayName = "Plant"),
	Undead UMETA(DisplayName = "Undead"),
	Aquatic UMETA(DisplayName = "Aquatic"),
	Demon UMETA(DisplayName = "Demon")
};
