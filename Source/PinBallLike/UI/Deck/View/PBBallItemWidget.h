// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallItemViewData.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallItemWidget.generated.h"

class UImage;
class UDragDropOperation;
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
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	void RefreshBallItem();

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UImage> Image_ProgressBar;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UImage> Image_Ball;

	UPROPERTY(Transient)
	FPBBallItemViewData ViewData;
};
