#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBBallSkillTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	int32 CalculateBaseDamage(int32 BallAttackPower) const;
	FText GetDescription(int32 BallAttackPower) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "0"))
	float PowerValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "0"))
	float LifeValue = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "0"))
	int32 EffectValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "0"))
	int32 GroggyValue = 0;
};
