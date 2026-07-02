#pragma once

#include "CoreMinimal.h"
#include "PBBallDeckSlot.generated.h"

UENUM(BlueprintType)
enum class EPBBallDeckSlotType : uint8
{
	Bench,
	Deployment
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDeckSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	EPBBallDeckSlotType SlotType = EPBBallDeckSlotType::Bench;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck")
	int32 BallInstanceId = INDEX_NONE;

	bool IsEmpty() const
	{
		return BallInstanceId == INDEX_NONE;
	}
};
