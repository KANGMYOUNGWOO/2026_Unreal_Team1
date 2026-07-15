// Fill out your copyright notice in the Description page of Project Settings.


#include "Ball/PBBallTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

using namespace PBSheetParserUtils;

UPBBallTableParser::UPBBallTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Blueprints/Ball/DataAsset");
	DataAssetPreset.NameFormat = TEXT("DA_{0}");
	IconPreset.FolderPath.Path = TEXT("/Game/Resources/Ball");
	IconPreset.NameFormat = TEXT("Icon_{0}");
	ActorPreset.FolderPath.Path = TEXT("/Game/Blueprints/Ball/Actor");
	ActorPreset.NameFormat = TEXT("BP_{0}");
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
	NewRow.RaceTypes = ParseEnumArray<EPBBallRaceType>(RowData.FindRef(TEXT("RaceTypes")));
	NewRow.ClassType = ParseEnumValue(RowData.FindRef(TEXT("ClassType")), EPBBallClassType::Attacker);
	NewRow.SynergyIds = ParseNameArray(RowData.FindRef(TEXT("SynergyIds")));
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
	
	if (ActorPreset.IsValid())
	{
		BallDataAsset->ActorClass = FindBlueprintClass<APBBallBase>(ActorPreset, RowName);
	}

	(void)BallDataAsset->MarkPackageDirty();
	return BallDataAsset;
}
