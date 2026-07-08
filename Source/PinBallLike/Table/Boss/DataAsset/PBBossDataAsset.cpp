// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBossDataAsset.h"

#include "PinBallLike/Table/Boss/PBBossAssetIds.h"

FPrimaryAssetId UPBBossDataAsset::GetPrimaryAssetId() const
{
	const FName AssetName = RowName.IsNone() ? GetFName() : RowName;
	return FPrimaryAssetId(PBBossAssetIds::Type::BossData, AssetName);
}
