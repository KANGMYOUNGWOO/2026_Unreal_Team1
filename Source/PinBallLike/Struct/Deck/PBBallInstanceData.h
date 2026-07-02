#pragma once

#include "CoreMinimal.h"
#include "PBBallInstanceData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	int32 InstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	int32 BallId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck", meta = (ClampMin = "1", UIMin = "1"))
	int32 StarLevel = 1;

	bool IsValid() const
	{
		return InstanceId != INDEX_NONE && BallId != 0;
	}
};
