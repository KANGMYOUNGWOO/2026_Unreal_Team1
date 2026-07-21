#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PBBallDetailTooltipViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBallDetailTooltipViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|DetailTooltipViewModel")
	void SetTooltipViewData(const FPBBallDetailTooltipViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Ball|DetailTooltipViewModel")
	void ClearTooltip();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	bool bHasBall = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FName BallId = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	TObjectPtr<UTexture2D> BallIconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FText BallNameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FText BallDescriptionText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FPBBallDetailInfoRowViewData HpRow;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FPBBallDetailInfoRowViewData MpRow;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FPBBallDetailInfoRowViewData AttackRow;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FPBBallDetailInfoRowViewData ManaRegenRow;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FPBBallDetailIconTextViewData PowerFlipData;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FPBBallDetailIconTextViewData ClassData;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	TArray<FPBBallDetailIconTextViewData> RaceDataList;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	TObjectPtr<UTexture2D> SkillIconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FText SkillNameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|DetailTooltipViewModel")
	FText SkillDescriptionText = FText::GetEmpty();
};
