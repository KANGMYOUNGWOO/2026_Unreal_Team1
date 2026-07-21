// Fill out your copyright notice in the Description page of Project Settings.


#include "Ball/PBBallTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

using namespace PBSheetParserUtils;

UPBBallTableParser::UPBBallTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Ball");
	DataAssetPreset.NameFormat = TEXT("DA_{0}");
	IconPreset.FolderPath.Path = TEXT("/Game/Resources/Ball/Icon");
	IconPreset.NameFormat = TEXT("Icon_{0}");
	SpritePreset.FolderPath.Path = TEXT("/Game/Resources/Ball/Sprite");
	SpritePreset.NameFormat = TEXT("Sprite_{0}");
}

const TCHAR* UPBBallTableParser::GetParserName() const
{
	return TEXT("Ball");
}

UScriptStruct* UPBBallTableParser::GetRowStruct() const
{
	return FPBBallTableRow::StaticStruct();
}

bool UPBBallTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBallTableRow NewRow;
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.DescriptionKey = FText::FromString(RowData.FindRef(TEXT("DescriptionKey")));
	NewRow.PowerFlipType = ParseEnumValue(RowData.FindRef(TEXT("PowerFlipType")), EPBPowerFlipType::Sword);
	NewRow.RaceTypes = ParseEnumArray<EPBBallRaceType>(RowData.FindRef(TEXT("RaceType")));

	const FString ClassTypeText = TrimCell(RowData.FindRef(TEXT("ClassType")));
	NewRow.ClassType = ParseEnumValue(ClassTypeText, EPBBallClassType::None);
	if (NewRow.ClassType == EPBBallClassType::None
		&& !ClassTypeText.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Sheet][Ball] Invalid or empty ClassType. RowName=%s ClassType=%s"),
			*RowName.ToString(),
			*ClassTypeText);
	}

	NewRow.DefaultSkillIds = ParseNameArray(RowData.FindRef(TEXT("DefaultSkillIds")));
	NewRow.StarLevelId = FName(*TrimCell(RowData.FindRef(TEXT("StarLevelId"))));
	NewRow.ShopId = FName(*TrimCell(RowData.FindRef(TEXT("ShopId"))));

	(void)SetupBallDataAsset(RowName);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBallDataAsset* UPBBallTableParser::SetupBallDataAsset(const FName RowName) const
{
	UPBBallDataAsset* BallDataAsset =
		GetOrCreateDataAsset<UPBBallDataAsset>(DataAssetPreset, RowName, TEXT("Ball"));
	if (!IsValid(BallDataAsset))
	{
		return nullptr;
	}

	if (IconPreset.IsValid())
	{
		BallDataAsset->Icon = FindObject<UTexture2D>(IconPreset, RowName);
	}

	if (SpritePreset.IsValid())
	{
		BallDataAsset->Sprite = FindObject<UTexture2D>(SpritePreset, RowName);
	}

	(void)BallDataAsset->MarkPackageDirty();
	return BallDataAsset;
}
