// Fill out your copyright notice in the Description page of Project Settings.

#include "Relic/PBRelicTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Table/Relic/DataAsset/PBRelicDataAsset.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PinBallLike/Struct/Relic/PBRelicTypes.h"

using namespace PBSheetParserUtils;

UPBRelicTableParser::UPBRelicTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Relic");
	DataAssetPreset.NameFormat = TEXT("DA_{0}");

	RelicIconPreset.FolderPath.Path = TEXT("/Game/Resources/Relic/Icon");
	RelicIconPreset.NameFormat = TEXT("Icon_{0}");
}

const TCHAR* UPBRelicTableParser::GetParserName() const
{
	return TEXT("Relic");
}

UScriptStruct* UPBRelicTableParser::GetRowStruct() const
{
	return FPBRelicTableRow::StaticStruct();
}

bool UPBRelicTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBRelicTableRow NewRow;
	UE_LOG(LogTemp, Warning, TEXT("===== Parse Start : %s ====="), *RowName.ToString());
	NewRow.DisplayName =
		FText::FromString(
			RowData.FindRef(TEXT("DisplayName")));

	NewRow.Description =
		FText::FromString(
			RowData.FindRef(TEXT("Description")));

	NewRow.Rarity =
		ParseEnumValue(
			RowData.FindRef(TEXT("Rarity")),
			EPBRelicRarity::Common);
	NewRow.EffectId = ParseNameValue(RowData.FindRef(TEXT("EffectId")));

	NewRow.RelicDataAsset = SetupRelicDataAsset(RowName, NewRow);

	TargetTable->AddRow(RowName, NewRow);

	UE_LOG(LogTemp, Warning, TEXT("===== Parse End : %s ====="), *RowName.ToString());
	return true;
	
}

UPBRelicDataAsset* UPBRelicTableParser::SetupRelicDataAsset(
	const FName RowName,
	const FPBRelicTableRow& RelicRow) const
{
	UPBRelicDataAsset* RelicDataAsset =
		GetOrCreateDataAsset<UPBRelicDataAsset>(
			DataAssetPreset,
			RowName,
			TEXT("Relic"));

	if (!IsValid(RelicDataAsset))
	{
		return nullptr;
	}

	if (RelicIconPreset.IsValid())
	{
		RelicDataAsset->RelicIcon =
			FindObject<UTexture2D>(RelicIconPreset, RowName);
	}

	RelicDataAsset->MarkPackageDirty();

	return RelicDataAsset;
}
