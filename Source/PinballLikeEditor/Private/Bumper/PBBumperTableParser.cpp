// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"

using namespace PBSheetParserUtils;

UPBBumperTableParser::UPBBumperTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper");
	DataAssetPreset.NameFormat = TEXT("DA_Bumper_{0}");
	IconPreset.NameFormat = TEXT("T_{0}");
}

const TCHAR* UPBBumperTableParser::GetParserName() const
{
	return TEXT("Bumper");
}

UScriptStruct* UPBBumperTableParser::GetRowStruct() const
{
	return FPBBumperTableRow::StaticStruct();
}

bool UPBBumperTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBumperTableRow NewRow;
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));
	NewRow.BumperType = ParseEnumValue(RowData.FindRef(TEXT("BumperType")), EPBBumperType::Rebound);
	NewRow.TriggerID = FName(*TrimCell(RowData.FindRef(TEXT("TriggerID"))));
	NewRow.RequiredTriggerCount = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("RequireTriggerCount")), 1),
		1);
	NewRow.EffectID = FName(*TrimCell(RowData.FindRef(TEXT("EffectID"))));
	if (UPBBumperDataAsset* BumperDataAsset = SetupBumperDataAsset(RowName))
	{
		NewRow.BumperDataAsset = TSoftObjectPtr<UPBBumperDataAsset>(BumperDataAsset);
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBumperDataAsset* UPBBumperTableParser::SetupBumperDataAsset(const FName RowName) const
{
	UPBBumperDataAsset* BumperDataAsset =
		GetOrCreateDataAsset<UPBBumperDataAsset>(DataAssetPreset, RowName, TEXT("Bumper"));
	if (!IsValid(BumperDataAsset))
	{
		return nullptr;
	}

	if (IconPreset.IsValid())
	{
		BumperDataAsset->Icon = FindObject<UTexture2D>(IconPreset, RowName);
	}

	(void)BumperDataAsset->MarkPackageDirty();
	return BumperDataAsset;
}
