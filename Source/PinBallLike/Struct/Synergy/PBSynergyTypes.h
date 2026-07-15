#pragma once

#include "CoreMinimal.h"
#include "PBSynergyTypes.generated.h"

UENUM(BlueprintType)
enum class EPBSynergyKind : uint8
{
	Race UMETA(DisplayName = "Race"),
	Class UMETA(DisplayName = "Class")
};

UENUM(BlueprintType)
enum class EPBSynergyRuleType : uint8
{
	Stat UMETA(DisplayName = "Stat"),
	Rule UMETA(DisplayName = "Rule"),
	Hybrid UMETA(DisplayName = "Hybrid")
};
