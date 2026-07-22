#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Struct/Relic/PBRelicTypes.h"
#include "PBRelicViewData.generated.h"

UENUM(BlueprintType)
enum class EPBRelicChoiceRewardType : uint8
{
	Relic,
	Gold
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBRelicViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View")
	EPBRelicChoiceRewardType RewardType = EPBRelicChoiceRewardType::Relic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View")
	FName RelicId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View")
	EPBRelicRarity Rarity = EPBRelicRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|View", meta = (ClampMin = "0"))
	int32 GoldAmount = 0;

	bool IsValid() const
	{
		return RewardType == EPBRelicChoiceRewardType::Gold || !RelicId.IsNone();
	}
};
