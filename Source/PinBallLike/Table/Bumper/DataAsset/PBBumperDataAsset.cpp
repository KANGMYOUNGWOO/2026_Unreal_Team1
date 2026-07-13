// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperDataAsset.h"

#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"

FPrimaryAssetId UPBBumperDataAsset::GetPrimaryAssetId() const
{
	// 에셋 파일명과 시트 RowName을 분리하기 위해 RowName을 우선 사용함
	const FName AssetName = RowName.IsNone() ? GetFName() : RowName;
	return FPrimaryAssetId(PBBumperAssetIds::Type::BumperData, AssetName);
}

#if WITH_EDITOR
bool UPBBumperDataAsset::SetRowNameForImport(const FName InRowName)
{
	if (InRowName.IsNone())
	{
		return false;
	}

	RowName = InRowName;
	return MarkPackageDirty();
}
#endif
