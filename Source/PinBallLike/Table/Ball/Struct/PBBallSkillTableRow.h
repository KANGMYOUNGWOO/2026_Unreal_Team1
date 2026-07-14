#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBBallSkillTableRow.generated.h"

class APBBallSkillActorBase;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "0.01"))
	float Duration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill", meta = (ClampMin = "1"))
	int32 DamageCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill")
	TSubclassOf<APBBallSkillActorBase> ActorClass;
};
