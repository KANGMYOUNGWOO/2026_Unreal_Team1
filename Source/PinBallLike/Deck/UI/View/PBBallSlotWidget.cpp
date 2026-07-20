// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallSlotWidget.h"

#include "PBBallItemWidget.h"
#include "PBBallDragDropOperation.h"
#include "Components/Overlay.h"
#include "Engine/World.h"
#include "PinBallLike/Deck/UI/ViewModel/PBBallSlotViewModel.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "View/MVVMView.h"

void UPBBallSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureSlotViewModel();
}

void UPBBallSlotWidget::SetSlotContext(EPBBallDeckSlotType InSlotType, int32 InSlotIndex)
{
	SlotType = InSlotType;
	SlotIndex = InSlotIndex;
	EnsureSlotViewModel();
	if (SlotViewModel)
	{
		SlotViewModel->SetSlotContext(SlotType);
	}

	if (BallItemWidget)
	{
		BallItemWidget->SetSourceSlot(SlotType, SlotIndex);
	}
}

void UPBBallSlotWidget::SetBallInstanceId(int32 InBallInstanceId)
{
	EnsureSlotViewModel();
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
			if (SlotViewModel)
			{
				SlotViewModel->SetHasBall(true);
			}

			UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget SetBallInstanceId built ViewData. Slot=%s SlotType=%d SlotIndex=%d BallInstanceId=%d BallId=%s Icon=%s"),
				*GetNameSafe(this),
				static_cast<int32>(SlotType),
				SlotIndex,
				ViewData.BallInstanceId,
				*ViewData.BallId.ToString(),
				*GetNameSafe(ViewData.Icon));
			BallItem->InitializeBallItem(ViewData);
		}
		else
		{
			if (SlotViewModel)
			{
				SlotViewModel->ClearBall();
				SlotViewModel->SetSlotContext(SlotType);
			}

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
	EnsureSlotViewModel();
	BallInstanceId = INDEX_NONE;
	if (SlotViewModel)
	{
		SlotViewModel->ClearBall();
		SlotViewModel->SetSlotContext(SlotType);
	}

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

	const bool bConsumesBattleShiftCount =
		BallDragDropOperation->SourceSlotType != SlotType;
	APBBattleGameState* BattleGameState = GetWorld()
		? GetWorld()->GetGameState<APBBattleGameState>()
		: nullptr;
	if (bConsumesBattleShiftCount
		&& BattleGameState
		&& BattleGameState->GetBattleLevelPhase() == EPBBattleLevelPhase::BallDeployment
		&& !BattleGameState->HasRemainingBattleShiftCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeck] Move rejected. No remaining battle shift count. SourceType=%d SourceIndex=%d TargetType=%d TargetIndex=%d"),
			static_cast<int32>(BallDragDropOperation->SourceSlotType),
			BallDragDropOperation->SourceSlotIndex,
			static_cast<int32>(SlotType),
			SlotIndex);
		return false;
	}

	const bool bMoved = DeckSubsystem->MoveBallBetweenSlots(
		BallDragDropOperation->SourceSlotType,
		BallDragDropOperation->SourceSlotIndex,
		SlotType,
		SlotIndex);

	if (bMoved
		&& bConsumesBattleShiftCount
		&& BattleGameState
		&& BattleGameState->GetBattleLevelPhase() == EPBBattleLevelPhase::BallDeployment)
	{
		BattleGameState->ConsumeBattleShiftCount();
	}

	return bMoved;
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

void UPBBallSlotWidget::EnsureSlotViewModel()
{
	if (!SlotViewModel)
	{
		SlotViewModel = NewObject<UPBBallSlotViewModel>(this);
	}

	// if (SlotViewModel)
	// {
	// 	ApplyViewModelToWidget();
	// }
}

bool UPBBallSlotWidget::ApplyViewModelToWidget()
{
	if (!SlotViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(SlotViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallSlotWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SlotViewModel));
	}
	return bResult;
}
