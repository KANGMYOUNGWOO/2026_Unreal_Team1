// Fill out your copyright notice in the Description page of Project Settings.


#include "PBSellZoneWidget.h"

#include "PBBallDragDropOperation.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Deck/UI/ViewModel/PBSellZoneViewModel.h"
#include "PinBallLike/Struct/Deck/PBDeckDragMessage.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "View/MVVMView.h"

void UPBSellZoneWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureSellZoneViewModel();
	DragStartedHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBDeckDragStartedMessage>(
		GameplayTags::Event_UI_Deck_Drag_Started,
		this,
		&UPBSellZoneWidget::HandleBallItemDragStarted);
	DragEndedHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBDeckDragEndedMessage>(
		GameplayTags::Event_UI_Deck_Drag_Ended,
		this,
		&UPBSellZoneWidget::HandleBallItemDragEnded);

	SetSellEnabled(false);
}

void UPBSellZoneWidget::NativeDestruct()
{
	DragStartedHandle.Unregister();
	DragEndedHandle.Unregister();

	Super::NativeDestruct();
}

void UPBSellZoneWidget::SetSellEnabled(bool bInSellEnabled)
{
	bSellEnabled = bInSellEnabled;
	SetVisibility(bSellEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	EnsureSellZoneViewModel();
	if (SellZoneViewModel)
	{
		SellZoneViewModel->SetSellEnabled(bSellEnabled);
	}
}

void UPBSellZoneWidget::SetSellBallInstanceId(int32 InBallInstanceId)
{
	EnsureSellZoneViewModel();
	if (!bSellEnabled)
	{
		ClearSellBall();
		return;
	}

	UPBBallDeckSubsystem* DeckSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	const int32 SellPrice = DeckSubsystem ? DeckSubsystem->GetSellPrice(InBallInstanceId) : 0;
	if (SellZoneViewModel)
	{
		SellZoneViewModel->SetSellBall(InBallInstanceId, SellPrice);
	}
}

void UPBSellZoneWidget::ClearSellBall()
{
	EnsureSellZoneViewModel();
	if (SellZoneViewModel)
	{
		SellZoneViewModel->ClearSellBall();
	}
}

bool UPBSellZoneWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
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

	if (!bSellEnabled || !SellZoneViewModel || SellZoneViewModel->BallInstanceId != BallDragDropOperation->BallInstanceId)
	{
		return false;
	}

	int32 SellPrice = 0;
	const bool bSold = DeckSubsystem->SellBall(BallDragDropOperation->BallInstanceId, SellPrice);
	ClearSellBall();
	return bSold;
}

void UPBSellZoneWidget::HandleBallItemDragStarted(FGameplayTag, const FPBDeckDragStartedMessage& Message)
{
	SetSellEnabled(true);
	SetSellBallInstanceId(Message.ItemId);
}

void UPBSellZoneWidget::HandleBallItemDragEnded(FGameplayTag, const FPBDeckDragEndedMessage&)
{
	SetSellEnabled(false);
}

void UPBSellZoneWidget::EnsureSellZoneViewModel()
{
	if (!SellZoneViewModel)
	{
		SellZoneViewModel = NewObject<UPBSellZoneViewModel>(this);
	}

	if (SellZoneViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBSellZoneWidget::ApplyViewModelToWidget()
{
	if (!SellZoneViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("SellZoneWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(SellZoneViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("SellZoneWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SellZoneViewModel));
	}
	return bResult;
}
