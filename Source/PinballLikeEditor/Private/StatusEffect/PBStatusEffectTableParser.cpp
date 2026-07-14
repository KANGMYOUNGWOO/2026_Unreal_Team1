// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusEffect/PBStatusEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Table/StatusEffect/DataAsset/PBStatusEffectDataAsset.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectRow.h"

using namespace PBSheetParserUtils;

UPBStatusEffectTableParser::UPBStatusEffectTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/StatusEffect");
	DataAssetPreset.NameFormat = TEXT("DA_StatusEffect_{0}");
	IconPreset.FolderPath.Path = TEXT("/Game/Resources/StatusEffect");
	IconPreset.NameFormat = TEXT("Icon_{0}");
}

const TCHAR* UPBStatusEffectTableParser::GetParserName() const
{
	return TEXT("StatusEffect");
}

UScriptStruct* UPBStatusEffectTableParser::GetRowStruct() const
{
	return FPBStatusEffectRow::StaticStruct();
}

bool UPBStatusEffectTableParser::ParseRow(FName RowName, const TMap<FString, FString>& RowData)
{
	FPBStatusEffectRow NewRow;
	NewRow.DisplayNameKey = FName(*TrimCell(RowData.FindRef(TEXT("DisplayNameKey"))));
	NewRow.DescriptionKey = FName(*TrimCell(RowData.FindRef(TEXT("DescriptionKey"))));
	NewRow.Tags = TrimCell(RowData.FindRef(TEXT("Tags")));
	NewRow.StackType = ParseEnumValue(RowData.FindRef(TEXT("StackType")), EPBStatusEffectStackType::Replace);
	NewRow.DurationPolicy = ParseEnumValue(RowData.FindRef(TEXT("DurationPolicy")), EPBStatusEffectDurationPolicy::Permanent);
	NewRow.DurationValue = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("DurationValue")), 0.0f), 0.0f);
	NewRow.Interval = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("Interval")), 0.0f), 0.0f);

	(void)SetupStatusEffectDataAsset(RowName, RowData);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBStatusEffectDataAsset* UPBStatusEffectTableParser::SetupStatusEffectDataAsset(
	const FName RowName,
	const TMap<FString, FString>& RowData) const
{
	UPBStatusEffectDataAsset* StatusEffectDataAsset =
		GetOrCreateDataAsset<UPBStatusEffectDataAsset>(DataAssetPreset, RowName, TEXT("StatusEffect"));
	if (!IsValid(StatusEffectDataAsset))
	{
		return nullptr;
	}

	if (IconPreset.IsValid())
	{
		const FString IconIdString = TrimCell(RowData.FindRef(TEXT("Icon")));
		const FName IconId = IsUnsetValue(IconIdString) ? RowName : FName(*IconIdString);
		StatusEffectDataAsset->Icon = FindObject<UTexture2D>(IconPreset, IconId);
	}

	(void)StatusEffectDataAsset->MarkPackageDirty();
	return StatusEffectDataAsset;
}
