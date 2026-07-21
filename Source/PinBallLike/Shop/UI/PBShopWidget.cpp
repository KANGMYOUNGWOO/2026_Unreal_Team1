// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopWidget.h"

#include "PBPurchaseConfirmWidget.h"
#include "PBShopRefuseWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components//Button.h"
#include "Components/CanvasPanelSlot.h"
#include "PBShopSlotWidget.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "PinBallLike/Struct/Choice/PBChoiceType.h"

void UPBShopWidget::UpdateSlotWidgetPositionsOnce()
{
	APlayerController* PC = GetOwningPlayer();
	
	if (!PC)
	{
		return;
	}
	
	const int32 Count = FMath::Min(ShopSlotWorldLocations.Num(),ShopSlotWidgets.Num());
	
	for (int32 i =0; i < Count; i++)
	{
		UPBShopSlotWidget* Slotwidget = ShopSlotWidgets[i];
		
		if (!Slotwidget)
		{
			continue;
		}
		
		const FVector TargetWorldLocation =
			ShopSlotWorldLocations[i] + SlotWidgetWorldOffset;
		
		FVector2D WidgetPosition;
		
		const bool IsProjected =
			UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
				PC,
				TargetWorldLocation,
				WidgetPosition,
				true);
		
		Slotwidget->SetVisibility(IsProjected ? 
			ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		
		if (!IsProjected)
		{
			continue;
		}
		
		UCanvasPanelSlot* CanvasSlot =
			Cast<UCanvasPanelSlot>(Slotwidget->Slot);
		
		if (!CanvasSlot)
		{
			continue;
		}
		
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		//CanvasSlot->SetPosition(WidgetPosition + FVector2D(0.f, 0));
		CanvasSlot->SetPosition(WidgetPosition);
		
		
	}
	
}

void UPBShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ShopSlotWidgets = 
	{
		Slot0,
		Slot1,
		Slot2,
		Slot3,
		Slot4,
		Slot5,
		Slot6,
		Slot7
	};
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this,
			&UPBShopWidget::OnExitButtonClicked);
	}
	
	if (Button_Reroll)
	{
		Button_Reroll->OnClicked.AddDynamic(this, &UPBShopWidget::OnRerollButtonClicked);
	}
	
	UpdateSlotWidgetPositionsOnce();
}

void UPBShopWidget::SetShopSlotWorldLocations(const TArray<FVector>& InWorldLocations)
{
	ShopSlotWorldLocations = InWorldLocations;

	// 바로 갱신하지 말고 다음 Tick에서 갱신
	bPendingUpdateSlotPositions = true;
	
	
}

void UPBShopWidget::SetShopSlotWidgetData(TArray<const FBallDataStruct*> BallDatas)
{
	for(int i=0; i<BallDatas.Num(); i++)
	{
		
	}
	
}

void UPBShopWidget::SetShopSlotWidgetData(int32 index, FText Name, int32 Price, FText Synergy)
{
	if (!ShopSlotWidgets.IsValidIndex(index) || !ShopSlotWidgets[index])
	{
		return;
	}

	ShopSlotWidgets[index]->SetVisibility(ESlateVisibility::Visible);
	ShopSlotWidgets[index]->SetSlotInfo(Name,Price,Synergy); 
	
}

void UPBShopWidget::UnActiveSlotWidget(int32 SlotIndex)
{
	if (!ShopSlotWidgets.IsValidIndex(SlotIndex) || !ShopSlotWidgets[SlotIndex])
	{
		return;
	}

	ShopSlotWidgets[SlotIndex]->SetVisibility(ESlateVisibility::Collapsed);
	ConfirmWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UPBShopWidget::RefuseWidgetSpawn()
{
	if (ConfirmWidget)
	{
		ConfirmWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if(RefuseWidget)
	{
		RefuseWidget->Show();
	}
}

void UPBShopWidget::SetPurchaseConfirmInfo(const FPBPurchaseConfirmData& Data)
{
	ConfirmWidget->SetInfo(Data.SlotIndex, Data.BallName,Data.Price,Data.SynergyDescription,nullptr);
	ConfirmWidget->SetVisibility(ESlateVisibility::Visible);
}

void UPBShopWidget::OnExitButtonClicked()
{
	FPBChoiceType Message;
	Message.Exit = 0;
	
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,Message);
	
	//RemoveFromParent();
}

void UPBShopWidget::OnRerollButtonClicked()
{
	OnRerollRequested.Broadcast();
}

void UPBShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bPendingUpdateSlotPositions)
	{
		bPendingUpdateSlotPositions = false;
		UpdateSlotWidgetPositionsOnce();
	}
}



