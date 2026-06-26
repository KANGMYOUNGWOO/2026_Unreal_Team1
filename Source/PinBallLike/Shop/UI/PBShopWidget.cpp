// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "PBShopSlotWidget.h"
#include "PinBallLike/Subsystem/BallDataStruct.h"


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
		
		CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		CanvasSlot->SetPosition(WidgetPosition + FVector2D(0.f, -40.f));
		CanvasSlot->SetPosition(WidgetPosition);
		
		UE_LOG(LogTemp, Warning, TEXT("Slot %d Projected=%d Pos=%s World=%s"),
	i,
	IsProjected,
	*WidgetPosition.ToString(),
	*TargetWorldLocation.ToString());
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
	
	UpdateSlotWidgetPositionsOnce();
}

void UPBShopWidget::SetShopSlotWorldLocations(const TArray<FVector>& InWorldLocations)
{
	ShopSlotWorldLocations = InWorldLocations;
	
	UpdateSlotWidgetPositionsOnce();
}

void UPBShopWidget::SetShopSlotWidgetData(TArray<const FBallDataStruct*> BallDatas)
{
	for(int i=0; i<BallDatas.Num(); i++)
	{
		
	}
	
}

void UPBShopWidget::SetShopSlotWidgetData(int32 index, FText Name, int32 Price, FText Synergy)
{
	ShopSlotWidgets[index]->SetSlotInfo(Name,Price,Synergy); 
	
}


