// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallSlotWidget.generated.h"

class UPBBallItemWidget;
class UDragDropOperation;
class UOverlay;
class UImage;

UCLASS()
class PINBALLLIKE_API UPBBallSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetSlotContext(EPBBallDeckSlotType InSlotType, int32 InSlotIndex);
	void SetBallInstanceId(int32 InBallInstanceId);
	void ClearBallItem();

	UFUNCTION(BlueprintPure, Category = "BallDeck")
	int32 GetBallInstanceId() const { return BallInstanceId; }

	UFUNCTION(BlueprintPure, Category = "BallDeck")
	EPBBallDeckSlotType GetSlotType() const { return SlotType; }

	UFUNCTION(BlueprintPure, Category = "BallDeck")
	int32 GetSlotIndex() const { return SlotIndex; }

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UPBBallItemWidget* CreateBallItem();

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UOverlay> Overlay_Root;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UImage> Image_Panel;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallItemWidget> BallItemWidget;

	UPROPERTY(EditDefaultsOnly, Category = "BallDeck", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBBallItemWidget> BallItemWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BallDeck", meta = (AllowPrivateAccess = "true"))
	EPBBallDeckSlotType SlotType = EPBBallDeckSlotType::Bench;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BallDeck", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex = INDEX_NONE;

	int32 BallInstanceId = INDEX_NONE;
	
};
