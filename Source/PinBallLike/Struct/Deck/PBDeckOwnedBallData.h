#pragma once

#include "CoreMinimal.h"
#include "PBDeckOwnedBallData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBDeckOwnedBallData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	int32 InstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	FName BallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck", meta = (ClampMin = "1", UIMin = "1"))
	int32 StarLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck", meta = (ClampMin = "0"))
	float SavedMana = 0.0f;

	bool IsValid() const
	{
		return InstanceId != INDEX_NONE && !BallId.IsNone();
	}
};
