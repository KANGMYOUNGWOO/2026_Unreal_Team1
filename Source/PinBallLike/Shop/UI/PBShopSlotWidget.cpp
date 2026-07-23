// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopSlotWidget.h"
#include "Components/TextBlock.h"

void UPBShopSlotWidget::SetSlotInfo(FText NameText, int32 Price, FText /*SynergyText*/)
{
	if (NameTextBlock)
	{
		NameTextBlock->SetText(NameText);
	}
	
	if (PriceTextBlock)
	{
		FText ItemPrice = FText::Format(
			NSLOCTEXT("Shop", "SlotPrice", "{0} 골드"),
			FText::AsNumber(Price));
		PriceTextBlock->SetText(ItemPrice);
	}
	
	if (SynergyTextBlock)
	{
		SynergyTextBlock->SetText(FText::GetEmpty());
		SynergyTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	
	
}
