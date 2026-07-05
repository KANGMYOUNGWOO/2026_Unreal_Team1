// Fill out your copyright notice in the Description page of Project Settings.


#include "PBSellZoneViewModel.h"

void UPBSellZoneViewModel::SetSellEnabled(bool bInSellEnabled)
{
	UE_MVVM_SET_PROPERTY_VALUE(bSellEnabled, bInSellEnabled);
	if (!bSellEnabled)
	{
		ClearSellBall();
	}
}

void UPBSellZoneViewModel::SetSellBall(int32 InBallInstanceId, int32 InPrice)
{
	UE_MVVM_SET_PROPERTY_VALUE(BallInstanceId, InBallInstanceId);
	UE_MVVM_SET_PROPERTY_VALUE(Price, InPrice);
	UE_MVVM_SET_PROPERTY_VALUE(PriceText, FText::AsNumber(InPrice));
}

void UPBSellZoneViewModel::ClearSellBall()
{
	UE_MVVM_SET_PROPERTY_VALUE(BallInstanceId, INDEX_NONE);
	UE_MVVM_SET_PROPERTY_VALUE(Price, 0);
	UE_MVVM_SET_PROPERTY_VALUE(PriceText, FText::GetEmpty());
}

bool UPBSellZoneViewModel::CanSell() const
{
	return bSellEnabled && BallInstanceId != INDEX_NONE && Price > 0;
}
