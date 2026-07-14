// Fill out your copyright notice in the Description page of Project Settings.


#include "PBSynergyDataAsset.h"
#include "PinBallLike/Table/Synergy/PBSynergyAssetIds.h"

FPrimaryAssetId UPBSynergyDataAsset::GetPrimaryAssetId() const
{
	const FName AssetName = RowName.IsNone() ? GetFName() : RowName;
	return FPrimaryAssetId(PBSynergyAssetIds::Type::SynergyData, AssetName);
}