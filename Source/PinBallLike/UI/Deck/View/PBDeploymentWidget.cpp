// Fill out your copyright notice in the Description page of Project Settings.


#include "PBDeploymentWidget.h"

#include "Components/UniformGridPanel.h"
#include "PBBallSlotWidget.h"
#include "PinBallLike/Subsystem/PBBallDeckSubsystem.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"

void UPBDeploymentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DeckSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;

	BuildDeploymentSlots();
	BindDeckEvents();
	RefreshDeploymentSlots();
}

void UPBDeploymentWidget::NativeDestruct()
{
	UnbindDeckEvents();
	Super::NativeDestruct();
}

void UPBDeploymentWidget::BuildDeploymentSlots()
{
	DeploymentSlotWidgets.Reset();

	if (!GridPanel_BallList)
	{
		return;
	}

	GridPanel_BallList->ClearChildren();

	if (!WBP_BallSlotClass)
	{
		return;
	}

	constexpr int32 DeploymentSlotCount = 3;
	DeploymentSlotWidgets.Reserve(DeploymentSlotCount);

	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlotCount; ++SlotIndex)
	{
		UPBBallSlotWidget* BallSlotWidget = CreateWidget<UPBBallSlotWidget>(this, WBP_BallSlotClass);
		if (!BallSlotWidget)
		{
			continue;
		}

		BallSlotWidget->SetSlotContext(EPBBallDeckSlotType::Deployment, SlotIndex);

		GridPanel_BallList->AddChildToUniformGrid(BallSlotWidget, 0, SlotIndex);
		DeploymentSlotWidgets.Add(BallSlotWidget);
	}
}

void UPBDeploymentWidget::BindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.AddDynamic(this, &UPBDeploymentWidget::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddDynamic(this, &UPBDeploymentWidget::HandleDeploymentSlotsReordered);
	DeckSubsystem->OnDeploymentSlotsRotated.AddDynamic(this, &UPBDeploymentWidget::HandleDeploymentSlotsRotated);
}

void UPBDeploymentWidget::UnbindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(this, &UPBDeploymentWidget::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(this, &UPBDeploymentWidget::HandleDeploymentSlotsReordered);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(this, &UPBDeploymentWidget::HandleDeploymentSlotsRotated);
}

void UPBDeploymentWidget::RefreshDeploymentSlots()
{
	if (!DeckSubsystem)
	{
		for (UPBBallSlotWidget* BallSlotWidget : DeploymentSlotWidgets)
		{
			if (BallSlotWidget)
			{
				BallSlotWidget->ClearBallItem();
			}
		}
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlotWidgets.Num(); ++SlotIndex)
	{
		if (UPBBallSlotWidget* BallSlotWidget = DeploymentSlotWidgets[SlotIndex])
		{
			BallSlotWidget->SetBallInstanceId(DeckSubsystem->GetDeploymentSlotBallInstanceId(SlotIndex));
		}
	}
}

void UPBDeploymentWidget::HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId)
{
	if (!DeploymentSlotWidgets.IsValidIndex(SlotIndex))
	{
		return;
	}

	if (UPBBallSlotWidget* BallSlotWidget = DeploymentSlotWidgets[SlotIndex])
	{
		BallSlotWidget->SetBallInstanceId(BallInstanceId);
	}
}

void UPBDeploymentWidget::HandleDeploymentSlotsReordered()
{
	RefreshDeploymentSlots();
}

void UPBDeploymentWidget::HandleDeploymentSlotsRotated()
{
	RefreshDeploymentSlots();
}
