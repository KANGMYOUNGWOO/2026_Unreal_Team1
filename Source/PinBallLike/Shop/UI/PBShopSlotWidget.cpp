// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopSlotWidget.h"
#include "Components/TextBlock.h"

void UPBShopSlotWidget::SetSlotInfo(FText NameText, int32 Price, FText SynergyText)
{
	if (NameTextBlock)
	{
		NameTextBlock->SetText(NameText);
	}
	
	if (PriceTextBlock)
	{
		PriceTextBlock->SetText(FText::AsNumber(Price));
	}
	
	if (SynergyTextBlock)
	{
		SynergyTextBlock->SetText(SynergyText);
	}
	
	
	
}
