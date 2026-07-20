#pragma once

#include "CoreMinimal.h"
#include "PBSynergyViewData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyBallIconViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	int32 BallInstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FName BallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	TObjectPtr<UTexture2D> Icon = nullptr;

	bool operator==(const FPBSynergyBallIconViewData& Other) const
	{
		return BallInstanceId == Other.BallInstanceId
			&& BallId == Other.BallId
			&& Icon == Other.Icon;
	}
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyTierViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	int32 RequiredCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText RequiredCountText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText EffectText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	bool bIsCurrentTier = false;

	bool operator==(const FPBSynergyTierViewData& Other) const
	{
		return RequiredCount == Other.RequiredCount
			&& RequiredCountText.EqualTo(Other.RequiredCountText)
			&& EffectText.EqualTo(Other.EffectText)
			&& bIsActive == Other.bIsActive
			&& bIsCurrentTier == Other.bIsCurrentTier;
	}
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSynergyViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FName SynergyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText SynergyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText CurrentCountText = FText::AsNumber(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	FText CountListText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	TArray<FPBSynergyTierViewData> TierViewDataList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|View")
	TArray<FPBSynergyBallIconViewData> BallIconViewDataList;

	bool IsValid() const
	{
		return !SynergyId.IsNone();
	}

	bool operator==(const FPBSynergyViewData& Other) const
	{
		return SynergyId == Other.SynergyId
			&& SynergyName.EqualTo(Other.SynergyName)
			&& Description.EqualTo(Other.Description)
			&& CurrentCount == Other.CurrentCount
			&& CurrentCountText.EqualTo(Other.CurrentCountText)
			&& CountListText.EqualTo(Other.CountListText)
			&& Icon == Other.Icon
			&& TierViewDataList == Other.TierViewDataList
			&& BallIconViewDataList == Other.BallIconViewDataList;
	}
};
