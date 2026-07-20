#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBBossTurtleFallingRockPattern.generated.h"

class APBTurtleFallingRock;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossTurtleFallingRockPattern : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock")
	TSubclassOf<APBTurtleFallingRock> FallingRockClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock", meta = (ClampMin = "0.0"))
	float FallingSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock", meta = (ClampMin = "1"))
	int32 FallingRockCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock", meta = (ClampMin = "0.0"))
	float SpawnHeight = 1000.0f;
};
