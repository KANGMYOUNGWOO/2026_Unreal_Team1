#pragma once

#include "CoreMinimal.h"
#include "PBBallDetailTooltipViewData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDetailInfoRowViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FText LabelText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FText ValueText = FText::GetEmpty();

	bool operator==(const FPBBallDetailInfoRowViewData& Other) const
	{
		return LabelText.EqualTo(Other.LabelText)
			&& ValueText.EqualTo(Other.ValueText);
	}
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDetailIconTextViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FText Text = FText::GetEmpty();

	bool operator==(const FPBBallDetailIconTextViewData& Other) const
	{
		return IconTexture == Other.IconTexture
			&& Text.EqualTo(Other.Text);
	}
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDetailTooltipViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	bool bHasBall = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FName BallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	TObjectPtr<UTexture2D> BallIconTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FText BallNameText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FText BallDescriptionText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FPBBallDetailInfoRowViewData HpRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FPBBallDetailInfoRowViewData MpRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FPBBallDetailInfoRowViewData AttackRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FPBBallDetailInfoRowViewData ManaRegenRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FPBBallDetailIconTextViewData PowerFlipData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FPBBallDetailIconTextViewData ClassData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	TArray<FPBBallDetailIconTextViewData> RaceDataList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	TObjectPtr<UTexture2D> SkillIconTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail")
	FText SkillNameText = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Detail", meta = (MultiLine = "true"))
	FText SkillDescriptionText = FText::GetEmpty();

	bool IsValid() const
	{
		return bHasBall && !BallId.IsNone();
	}

	bool operator==(const FPBBallDetailTooltipViewData& Other) const
	{
		return bHasBall == Other.bHasBall
			&& BallId == Other.BallId
			&& BallIconTexture == Other.BallIconTexture
			&& BallNameText.EqualTo(Other.BallNameText)
			&& BallDescriptionText.EqualTo(Other.BallDescriptionText)
			&& HpRow == Other.HpRow
			&& MpRow == Other.MpRow
			&& AttackRow == Other.AttackRow
			&& ManaRegenRow == Other.ManaRegenRow
			&& PowerFlipData == Other.PowerFlipData
			&& ClassData == Other.ClassData
			&& RaceDataList == Other.RaceDataList
			&& SkillIconTexture == Other.SkillIconTexture
			&& SkillNameText.EqualTo(Other.SkillNameText)
			&& SkillDescriptionText.EqualTo(Other.SkillDescriptionText);
	}
};
