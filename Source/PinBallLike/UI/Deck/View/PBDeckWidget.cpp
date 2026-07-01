// Fill out your copyright notice in the Description page of Project Settings.


#include "PBDeckWidget.h"

#include "Components/UniformGridPanel.h"
#include "PinBallLike/Subsystem/PBBallDeckSubsystem.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallSlotWidget.h"

void UPBDeckWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DeckSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;

	BuildBenchSlots();
	BindDeckEvents();
	RefreshBenchSlots();
}

void UPBDeckWidget::NativeDestruct()
{
	UnbindDeckEvents();
	Super::NativeDestruct();
}

void UPBDeckWidget::BuildBenchSlots()
{
	BenchSlotWidgets.Reset();

	if (!GridPanel_BallList)
	{
		return;
	}

	GridPanel_BallList->ClearChildren();

	if (!WBP_BallSlotClass)
	{
		return;
	}

	constexpr int32 BenchSlotCount = 9;
	constexpr int32 ColumnCount = 3;
	BenchSlotWidgets.Reserve(BenchSlotCount);

	for (int32 SlotIndex = 0; SlotIndex < BenchSlotCount; ++SlotIndex)
	{
		UPBBallSlotWidget* BallSlotWidget = CreateWidget<UPBBallSlotWidget>(this, WBP_BallSlotClass);
		if (!BallSlotWidget)
		{
			continue;
		}

		BallSlotWidget->SetSlotContext(EPBBallDeckSlotType::Bench, SlotIndex);

		const int32 Row = SlotIndex / ColumnCount;
		const int32 Column = SlotIndex % ColumnCount;
		GridPanel_BallList->AddChildToUniformGrid(BallSlotWidget, Row, Column);
		BenchSlotWidgets.Add(BallSlotWidget);
	}
}

void UPBDeckWidget::BindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnBenchSlotChanged.AddDynamic(this, &UPBDeckWidget::HandleBenchSlotChanged);
	DeckSubsystem->OnBenchSlotsSwapped.AddDynamic(this, &UPBDeckWidget::HandleBenchSlotsSwapped);
}

void UPBDeckWidget::UnbindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnBenchSlotChanged.RemoveDynamic(this, &UPBDeckWidget::HandleBenchSlotChanged);
	DeckSubsystem->OnBenchSlotsSwapped.RemoveDynamic(this, &UPBDeckWidget::HandleBenchSlotsSwapped);
}

void UPBDeckWidget::RefreshBenchSlots()
{
	if (!DeckSubsystem)
	{
		for (UPBBallSlotWidget* BallSlotWidget : BenchSlotWidgets)
		{
			if (BallSlotWidget)
			{
				BallSlotWidget->ClearBallItem();
			}
		}
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < BenchSlotWidgets.Num(); ++SlotIndex)
	{
		if (UPBBallSlotWidget* BallSlotWidget = BenchSlotWidgets[SlotIndex])
		{
			BallSlotWidget->SetBallInstanceId(DeckSubsystem->GetSlotBallInstanceId(EPBBallDeckSlotType::Bench, SlotIndex));
		}
	}
}

void UPBDeckWidget::HandleBenchSlotChanged(int32 SlotIndex, int32 BallInstanceId)
{
	if (!BenchSlotWidgets.IsValidIndex(SlotIndex))
	{
		return;
	}

	if (UPBBallSlotWidget* BallSlotWidget = BenchSlotWidgets[SlotIndex])
	{
		BallSlotWidget->SetBallInstanceId(BallInstanceId);
	}
}

void UPBDeckWidget::HandleBenchSlotsSwapped()
{
	RefreshBenchSlots();
}
