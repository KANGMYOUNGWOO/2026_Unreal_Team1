// Fill out your copyright notice in the Description page of Project Settings.


#include "PBStatusEffectDataAsset.h"

#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"
FPrimaryAssetId UPBStatusEffectDataAsset::GetPrimaryAssetId() const
{
	const FName AssetName = RowName.IsNone() ? GetFName() : RowName;
	return FPrimaryAssetId(PBStatusEffectAssetIds::Type::StatusEffectData, AssetName);
}
