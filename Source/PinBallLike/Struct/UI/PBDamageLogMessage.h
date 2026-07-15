#pragma once

#include "CoreMinimal.h"
#include "PBDamageLogMessage.generated.h"

UENUM(BlueprintType)
enum class EPBDamageLogType : uint8
{
	Default,
	Skill,
	Ball,
	Boss
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBDamageLogMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "UI|Damage Log")
	EPBDamageLogType LogType = EPBDamageLogType::Default;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Damage Log")
	int32 DamageAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Damage Log")
	FVector HitLocation = FVector::ZeroVector;
};
