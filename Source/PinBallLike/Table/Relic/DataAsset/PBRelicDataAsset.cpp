// Fill out your copyright notice in the Description page of Project Settings.


#include "PBRelicDataAsset.h"

#include "PinBallLike/Table/Relic/PBRelicAssetIds.h"

FPrimaryAssetId UPBRelicDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(
		PBRelicAssetIds::Type::RelicData,
		GetFName());
}