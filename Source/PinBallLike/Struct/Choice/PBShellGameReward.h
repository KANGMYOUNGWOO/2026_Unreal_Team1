#pragma once

#include "CoreMinimal.h"
#include "PBShellGameReward.generated.h"

UENUM(BlueprintType)
enum class EPBShellGameRewardType : uint8
{
	None,
	Ball,
	Gold
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBShellGameReward
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shell Game|Reward")
	EPBShellGameRewardType Type = EPBShellGameRewardType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Shell Game|Reward")
	FName BallId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Shell Game|Reward")
	int32 GoldAmount = 0;

	bool IsValid() const
	{
		switch (Type)
		{
		case EPBShellGameRewardType::Ball:
			return !BallId.IsNone();

		case EPBShellGameRewardType::Gold:
			return GoldAmount > 0;

		default:
			return false;
		}
	}
};
