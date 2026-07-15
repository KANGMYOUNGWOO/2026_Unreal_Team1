#pragma once

#include "CoreMinimal.h"
#include "PBSynergyViewData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FName SynergyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText SynergyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText CurrentCountText = FText::AsNumber(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText CountListText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	TObjectPtr<UTexture2D> Icon = nullptr;

	bool IsValid() const
	{
		return !SynergyId.IsNone();
	}

	bool operator==(const FPBSynergyViewData& Other) const
	{
		return SynergyId == Other.SynergyId
			&& SynergyName.EqualTo(Other.SynergyName)
			&& CurrentCount == Other.CurrentCount
			&& CurrentCountText.EqualTo(Other.CurrentCountText)
			&& CountListText.EqualTo(Other.CountListText)
			&& Icon == Other.Icon;
	}
};
