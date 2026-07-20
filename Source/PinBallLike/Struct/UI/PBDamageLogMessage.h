#pragma once

#include "CoreMinimal.h"
#include "PBDamageLogMessage.generated.h"

UENUM(BlueprintType)
enum class EPBDamageLogStyle : uint8
{
	Default,
	PlayerAttack,
	PlayerSkill,
	EnemyAttack
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBDamageLogMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "UI|Damage Log")
	EPBDamageLogStyle Style = EPBDamageLogStyle::Default;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Damage Log")
	int32 DamageAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Damage Log")
	FVector HitLocation = FVector::ZeroVector;
};
