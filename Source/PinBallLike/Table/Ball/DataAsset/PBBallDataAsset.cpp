// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDataAsset.h"

#include "PinBallLike/Table/Ball/PBBallAssetIds.h"

FPrimaryAssetId UPBBallDataAsset::GetPrimaryAssetId() const
{
	const FName AssetName = RowName.IsNone() ? GetFName() : RowName;
	return FPrimaryAssetId(PBBallAssetIds::Type::BallData, AssetName);
}
