// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "PBShopSlotWidget.h"



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
}

void UPBShopWidget::SetShopSlotWorldLocations(const TArray<FVector>& InWorldLocations)
{
	ShopSlotWorldLocations = InWorldLocations;
	
	UpdateSlotWidgetPositionsOnce();
}
