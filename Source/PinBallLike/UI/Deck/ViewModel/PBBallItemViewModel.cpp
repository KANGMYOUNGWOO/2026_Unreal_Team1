// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallItemViewModel.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"

void UPBBallItemViewModel::SetBall(APBBallBase* InBall)
{
	Ball = InBall;
	RefreshFromBall();
}

void UPBBallItemViewModel::ClearBall()
{
	Ball = nullptr;
	RefreshFromBall();
}

void UPBBallItemViewModel::RefreshFromBall()
{
	const bool bNewHasBall = IsValid(Ball);
	UE_MVVM_SET_PROPERTY_VALUE(bHasBall, bNewHasBall);

	if (!bNewHasBall)
	{
		UE_MVVM_SET_PROPERTY_VALUE(BallNameText, FText::GetEmpty());
		UE_MVVM_SET_PROPERTY_VALUE(BallDescriptionText, FText::GetEmpty());
		UE_MVVM_SET_PROPERTY_VALUE(EnergyPercent, 0.0f);
		return;
	}

	//UE_MVVM_SET_PROPERTY_VALUE(BallNameText, Ball->GetDisplayNameText());
	UE_MVVM_SET_PROPERTY_VALUE(BallDescriptionText, FText::GetEmpty());
	//UE_MVVM_SET_PROPERTY_VALUE(EnergyPercent, Ball->GetResourceRatio(PBResourceNames::Mana));
}
