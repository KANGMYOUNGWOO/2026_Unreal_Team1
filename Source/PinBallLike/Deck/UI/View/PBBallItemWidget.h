// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallItemViewData.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallItemWidget.generated.h"

class UDragDropOperation;
class UPBBallItemViewModel;

DECLARE_MULTICAST_DELEGATE_OneParam(FPBOnBallItemDragStarted, int32);
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBBallItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static FPBOnBallItemDragStarted OnBallItemDragStarted;

	UFUNCTION(BlueprintCallable, Category = "BallDeck")
	void InitializeBallItem(const FPBBallItemViewData& InViewData);
	void SetSourceSlot(EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	void EnsureItemViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	FPBBallItemViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallItemViewModel> ItemViewModel;
};
