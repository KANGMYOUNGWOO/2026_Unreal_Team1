// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopViewModel.h"

void UPBShopViewModel::SetGold(int32 NewGold)
{
	Gold = NewGold;

	UE_MVVM_SET_PROPERTY_VALUE(
		GoldText,
		FText::Format(
			FText::FromString(TEXT("현재 골드 : {0}")),
			Gold
		)
	);
}