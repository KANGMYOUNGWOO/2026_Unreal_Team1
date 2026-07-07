// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallSlotViewModel.h"

void UPBBallSlotViewModel::SetSlotContext(EPBBallDeckSlotType InSlotType)
{
	UE_MVVM_SET_PROPERTY_VALUE(SlotType, InSlotType);
}

void UPBBallSlotViewModel::SetHasBall(bool bInHasBall)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasBall, bInHasBall);
}

void UPBBallSlotViewModel::ClearBall()
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasBall, false);
}
