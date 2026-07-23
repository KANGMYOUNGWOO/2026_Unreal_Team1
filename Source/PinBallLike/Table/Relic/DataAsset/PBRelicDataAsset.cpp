// Fill out your copyright notice in the Description page of Project Settings.


#include "PBRelicDataAsset.h"

#include "PinBallLike/Table/Relic/PBRelicAssetIds.h"

FPrimaryAssetId UPBRelicDataAsset::GetPrimaryAssetId() const
{
	const FName AssetName = RowName.IsNone() ? GetFName() : RowName;
	return FPrimaryAssetId(PBRelicAssetIds::Type::RelicData, AssetName);
}
