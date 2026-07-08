// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallItemWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "InputCoreTypes.h"
#include "PBBallDragDropOperation.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Deck/UI/ViewModel/PBBallItemViewModel.h"
#include "PinBallLike/Struct/Deck/PBDeckDragMessage.h"
#include "View/MVVMView.h"

void UPBBallItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureItemViewModel();
}

void UPBBallItemWidget::InitializeBallItem(const FPBBallItemViewData& InViewData)
{
	ViewData = InViewData;
	EnsureItemViewModel();
	if (ItemViewModel)
	{
		ItemViewModel->SetBallItemViewData(ViewData);
	}

	UE_LOG(LogTemp, Warning, TEXT("BallItemWidget InitializeBallItem. Widget=%s BallInstanceId=%d BallId=%s StarLevel=%d SlotType=%d SlotIndex=%d Icon=%s"),
		*GetNameSafe(this),
		ViewData.BallInstanceId,
		*ViewData.BallId.ToString(),
		ViewData.StarLevel,
		static_cast<int32>(ViewData.SourceSlotType),
		ViewData.SourceSlotIndex,
		*GetNameSafe(ViewData.Icon));
}

void UPBBallItemWidget::SetSourceSlot(EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex)
{
	ViewData.SourceSlotType = InSourceSlotType;
	ViewData.SourceSlotIndex = InSourceSlotIndex;
	if (ItemViewModel)
	{
		ItemViewModel->SetBallItemViewData(ViewData);
	}
}

void UPBBallItemWidget::EnsureItemViewModel()
{
	if (!ItemViewModel)
	{
		ItemViewModel = NewObject<UPBBallItemViewModel>(this);
	}

	if (ItemViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBallItemWidget::ApplyViewModelToWidget()
{
	if (!ItemViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallItemWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ItemViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallItemWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ItemViewModel));
	}
	return bResult;
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
	DragDropOperation->Payload = this;

	if (UPBBallItemWidget* DragVisualWidget = CreateWidget<UPBBallItemWidget>(this, GetClass()))
	{
		DragVisualWidget->InitializeBallItem(ViewData);
		DragVisualWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		DragDropOperation->DefaultDragVisual = DragVisualWidget;
	}

	OutOperation = DragDropOperation;

	FPBDeckDragStartedMessage Message;
	Message.ItemId = ViewData.BallInstanceId;
	Message.SourceObject = this;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Deck_Drag_Started,
		Message);
}
