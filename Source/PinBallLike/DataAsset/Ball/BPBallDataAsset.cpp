// Fill out your copyright notice in the Description page of Project Settings.


#include "BPBallDataAsset.h"

const FPrimaryAssetType UPBBallDataAsset::BallDataAssetType = TEXT("BallData");

FPrimaryAssetId UPBBallDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(BallDataAssetType, GetFName());
}
