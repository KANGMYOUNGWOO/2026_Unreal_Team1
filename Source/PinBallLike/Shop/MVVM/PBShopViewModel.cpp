// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopViewModel.h"

int32 UPBShopViewModel::GetGold() const
{
	return Gold;
}

void UPBShopViewModel::SetGold(int32 NewGold)
{
	if (Gold == NewGold)
	{
		return;
	}

	Gold = NewGold;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetGold);
}
