// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallItemViewData.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallItemWidget.generated.h"

class UDragDropOperation;
class UTexture2D;
class UPBBallDetailTooltipWidget;
class UPBBallItemViewModel;
struct FPBBallDetailIconTextViewData;
struct FPBBallDetailInfoRowViewData;
struct FPBBallDetailTooltipViewData;
struct FPBBallSkillTableRow;
struct FPBBallStarLevelRow;
struct FPBBallTableRow;

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBBallItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BallDeck")
	void InitializeBallItem(const FPBBallItemViewData& InViewData);
	void SetSourceSlot(EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BallDeck|Tooltip")
	TSubclassOf<UPBBallDetailTooltipWidget> BallDetailTooltipWidgetClass;

private:
	void EnsureItemViewModel();
	bool ApplyViewModelToWidget();
	void RebuildBallDetailTooltip();
	bool BuildBallDetailTooltipViewData(FPBBallDetailTooltipViewData& OutTooltipViewData) const;
	FPBBallDetailInfoRowViewData MakeInfoRow(const FText& LabelText, int32 Value) const;
	FPBBallDetailIconTextViewData MakeIconText(UTexture2D* IconTexture, const FText& Text) const;
	int32 FindMapValue(const TMap<FName, int32>& Values, FName Key) const;
	FText GetEnumDisplayText(const UEnum* Enum, int64 Value) const;
	FName GetEnumValueName(const UEnum* Enum, int64 Value) const;

	UPROPERTY(Transient)
	FPBBallItemViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallItemViewModel> ItemViewModel;
};
