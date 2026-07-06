// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallItemViewModel.h"

void UPBBallItemViewModel::SetBallItemViewData(const FPBBallItemViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasBall, InViewData.IsValid());
	UE_MVVM_SET_PROPERTY_VALUE(StarLevel, InViewData.StarLevel);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InViewData.Icon);
	UE_MVVM_SET_PROPERTY_VALUE(Progress, CalculateStarProgress(InViewData.StarLevel));
}

void UPBBallItemViewModel::ClearBall()
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasBall, false);
	UE_MVVM_SET_PROPERTY_VALUE(StarLevel, 1);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, nullptr);
	UE_MVVM_SET_PROPERTY_VALUE(Progress, 1.0f);
}

float UPBBallItemViewModel::CalculateStarProgress(int32 InStarLevel) const
{
	constexpr float MaxStarLevel = 3.0f;
	const float ClampedStarLevel = FMath::Clamp(static_cast<float>(InStarLevel), 0.0f, MaxStarLevel);
	const float FilledRatio = ClampedStarLevel / MaxStarLevel;
	return 1.0f - FilledRatio;
}
