#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Component/PBBossHitPartComponent.h"
#include "PBBossHitPointData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBossHitPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitPoint")
	FName HitPointName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitPoint")
	EPBBossHitPartType HitPartType = EPBBossHitPartType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitPoint", meta = (ClampMin = "0"))
	int32 HPDamageMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitPoint", meta = (ClampMin = "0"))
	int32 GroggyMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitPoint")
	bool IsWeaknessPoint = false;
};
