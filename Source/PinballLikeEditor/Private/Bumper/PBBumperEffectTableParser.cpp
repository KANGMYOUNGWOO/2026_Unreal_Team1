// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperEffectDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"

using namespace PBSheetParserUtils;

UPBBumperEffectTableParser::UPBBumperEffectTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper/Effect");
	DataAssetPreset.NameFormat = TEXT("DA_Effect_{0}");
	EffectClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Effect");
	EffectClassPreset.NameFormat = TEXT("BP_{0}");
}

const TCHAR* UPBBumperEffectTableParser::GetParserName() const
{
	return TEXT("Effect");
}

UScriptStruct* UPBBumperEffectTableParser::GetRowStruct() const
{
	return FPBBumperEffectRow::StaticStruct();
}

bool UPBBumperEffectTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBumperEffectRow NewRow;
	NewRow.EffectType = ParseEnumValue(RowData.FindRef(TEXT("EffectType")), EPBBumperEffectType::Instant);
	NewRow.Power = ParseFloatValue(RowData.FindRef(TEXT("Power")), 0.0f);
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));
	if (UPBBumperEffectDataAsset* EffectDataAsset = SetupEffectDataAsset(RowName))
	{
		NewRow.EffectDataAsset = TSoftObjectPtr<UPBBumperEffectDataAsset>(EffectDataAsset);
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBumperEffectDataAsset* UPBBumperEffectTableParser::SetupEffectDataAsset(const FName RowName) const
{
	UPBBumperEffectDataAsset* EffectDataAsset =
		GetOrCreateDataAsset<UPBBumperEffectDataAsset>(DataAssetPreset, RowName, TEXT("Effect"));
	if (!IsValid(EffectDataAsset))
	{
		return nullptr;
	}

	EffectDataAsset->EffectClass =
		FindBlueprintClass<UPBBumperEffectBase>(EffectClassPreset, RowName);
	(void)EffectDataAsset->MarkPackageDirty();

	return EffectDataAsset;
}
