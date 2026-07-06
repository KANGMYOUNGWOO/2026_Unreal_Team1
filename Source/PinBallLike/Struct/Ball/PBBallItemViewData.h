#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallItemViewData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallItemViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|View")
	int32 BallInstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|View")
	int32 BallId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|View")
	int32 StarLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|View")
	EPBBallDeckSlotType SourceSlotType = EPBBallDeckSlotType::Bench;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|View")
	int32 SourceSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|View")
	TObjectPtr<UTexture2D> Icon = nullptr;

	bool IsValid() const
	{
		return BallInstanceId != INDEX_NONE && BallId != 0;
	}
};
