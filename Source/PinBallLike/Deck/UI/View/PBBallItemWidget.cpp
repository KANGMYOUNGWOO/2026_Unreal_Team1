// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallItemWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "InputCoreTypes.h"
#include "PBBallDragDropOperation.h"

void UPBBallItemWidget::InitializeBallItem(const FPBBallItemViewData& InViewData)
{
	ViewData = InViewData;
	UE_LOG(LogTemp, Warning, TEXT("BallItemWidget InitializeBallItem. Widget=%s BallInstanceId=%d BallId=%d StarLevel=%d SlotType=%d SlotIndex=%d Icon=%s"),
		*GetNameSafe(this),
		ViewData.BallInstanceId,
		ViewData.BallId,
		ViewData.StarLevel,
		static_cast<int32>(ViewData.SourceSlotType),
		ViewData.SourceSlotIndex,
		*GetNameSafe(ViewData.Icon));
	RefreshBallItem();
}

void UPBBallItemWidget::SetSourceSlot(EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex)
{
	ViewData.SourceSlotType = InSourceSlotType;
	ViewData.SourceSlotIndex = InSourceSlotIndex;
}

void UPBBallItemWidget::RefreshBallItem()
{
	if (Image_Ball)
	{
		if (ViewData.Icon)
		{
			Image_Ball->SetBrushFromTexture(ViewData.Icon, true);
			UE_LOG(LogTemp, Warning, TEXT("BallItemWidget RefreshBallItem set icon. Widget=%s Image=%s Icon=%s"),
				*GetNameSafe(this),
				*GetNameSafe(Image_Ball),
				*GetNameSafe(ViewData.Icon));
		}
		else
		{
			Image_Ball->SetBrush(FSlateBrush());
			UE_LOG(LogTemp, Warning, TEXT("BallItemWidget RefreshBallItem cleared icon. Widget=%s Image=%s BallInstanceId=%d BallId=%d"),
				*GetNameSafe(this),
				*GetNameSafe(Image_Ball),
				ViewData.BallInstanceId,
				ViewData.BallId);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BallItemWidget RefreshBallItem failed. Image_Ball is null. Widget=%s BallInstanceId=%d BallId=%d Icon=%s"),
			*GetNameSafe(this),
			ViewData.BallInstanceId,
			ViewData.BallId,
			*GetNameSafe(ViewData.Icon));
	}
}

FReply UPBBallItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (ViewData.BallInstanceId == INDEX_NONE)
	{
		return Reply;
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void UPBBallItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (ViewData.BallInstanceId == INDEX_NONE)
	{
		return;
	}

	UPBBallDragDropOperation* DragDropOperation = NewObject<UPBBallDragDropOperation>(this);
	if (!DragDropOperation)
	{
		return;
	}

	DragDropOperation->InitializeBallDrag(ViewData.BallInstanceId, ViewData.SourceSlotType, ViewData.SourceSlotIndex);
	DragDropOperation->Pivot = EDragPivot::MouseDown;

	if (UPBBallItemWidget* DragVisualWidget = CreateWidget<UPBBallItemWidget>(this, GetClass()))
	{
		DragVisualWidget->InitializeBallItem(ViewData);
		DragDropOperation->DefaultDragVisual = DragVisualWidget;
	}

	OutOperation = DragDropOperation;
}
