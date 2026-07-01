// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallSlotWidget.h"

#include "PBBallItemWidget.h"
#include "PBBallDragDropOperation.h"
#include "Components/Overlay.h"
#include "PinBallLike/Subsystem/PBBallDeckSubsystem.h"

void UPBBallSlotWidget::SetSlotContext(EPBBallDeckSlotType InSlotType, int32 InSlotIndex)
{
	SlotType = InSlotType;
	SlotIndex = InSlotIndex;

	if (BallItemWidget)
	{
		BallItemWidget->SetSourceSlot(SlotType, SlotIndex);
	}
}

void UPBBallSlotWidget::SetBallInstanceId(int32 InBallInstanceId)
{
	BallInstanceId = InBallInstanceId;

	if (BallInstanceId == INDEX_NONE)
	{
		ClearBallItem();
		return;
	}

	UPBBallItemWidget* BallItem = BallItemWidget.Get();
	if (!BallItem)
	{
		BallItem = CreateBallItem();
	}

	if (BallItem)
	{
		UPBBallDeckSubsystem* DeckSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
		FPBBallItemViewData ViewData;
		if (DeckSubsystem && DeckSubsystem->BuildBallItemViewData(BallInstanceId, SlotType, SlotIndex, ViewData))
		{
			UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget SetBallInstanceId built ViewData. Slot=%s SlotType=%d SlotIndex=%d BallInstanceId=%d BallId=%d Icon=%s"),
				*GetNameSafe(this),
				static_cast<int32>(SlotType),
				SlotIndex,
				ViewData.BallInstanceId,
				ViewData.BallId,
				*GetNameSafe(ViewData.Icon));
			BallItem->InitializeBallItem(ViewData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget SetBallInstanceId failed to build ViewData. Slot=%s SlotType=%d SlotIndex=%d BallInstanceId=%d DeckSubsystem=%s"),
				*GetNameSafe(this),
				static_cast<int32>(SlotType),
				SlotIndex,
				BallInstanceId,
				DeckSubsystem ? TEXT("valid") : TEXT("null"));
		}
	}
}

void UPBBallSlotWidget::ClearBallItem()
{
	BallInstanceId = INDEX_NONE;

	if (BallItemWidget)
	{
		BallItemWidget->RemoveFromParent();
		BallItemWidget = nullptr;
	}
}

bool UPBBallSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UPBBallDragDropOperation* BallDragDropOperation = Cast<UPBBallDragDropOperation>(InOperation);
	if (!BallDragDropOperation || !BallDragDropOperation->IsValidBallDrag())
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	UPBBallDeckSubsystem* DeckSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	if (!DeckSubsystem)
	{
		return false;
	}

	return DeckSubsystem->MoveBallBetweenSlots(
		BallDragDropOperation->SourceSlotType,
		BallDragDropOperation->SourceSlotIndex,
		SlotType,
		SlotIndex);
}

UPBBallItemWidget* UPBBallSlotWidget::CreateBallItem()
{
	if (!Overlay_Root || !BallItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget CreateBallItem failed. Slot=%s Overlay=%s Class=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Overlay_Root),
			*GetNameSafe(BallItemWidgetClass.Get()));
		return nullptr;
	}

	UPBBallItemWidget* NewBallItemWidget = CreateWidget<UPBBallItemWidget>(this, BallItemWidgetClass);
	if (!NewBallItemWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget CreateBallItem CreateWidget failed. Slot=%s Class=%s"),
			*GetNameSafe(this),
			*GetNameSafe(BallItemWidgetClass.Get()));
		return nullptr;
	}

	Overlay_Root->AddChildToOverlay(NewBallItemWidget);
	BallItemWidget = NewBallItemWidget;
	UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget CreateBallItem succeeded. Slot=%s Item=%s Class=%s"),
		*GetNameSafe(this),
		*GetNameSafe(NewBallItemWidget),
		*GetNameSafe(BallItemWidgetClass.Get()));
	return NewBallItemWidget;
}
