// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperTriggerTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperTriggerDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"

using namespace PBSheetParserUtils;

UPBBumperTriggerTableParser::UPBBumperTriggerTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper/Trigger");
	DataAssetPreset.NameFormat = TEXT("DA_Trigger_{0}");
	TriggerClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Trigger");
	TriggerClassPreset.NameFormat = TEXT("BP_{0}");
}

const TCHAR* UPBBumperTriggerTableParser::GetParserName() const
{
	return TEXT("Trigger");
}

UScriptStruct* UPBBumperTriggerTableParser::GetRowStruct() const
{
	return FPBBumperTriggerRow::StaticStruct();
}

bool UPBBumperTriggerTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBumperTriggerRow NewRow;
	NewRow.TriggerType = ParseEnumValue(RowData.FindRef(TEXT("TriggerType")), EPBBumperTriggerType::HitCount);
	NewRow.PositionIds = ParsePositionIds(RowData.FindRef(TEXT("PositionIds")));
	NewRow.TriggerDescription = FText::FromString(RowData.FindRef(TEXT("TriggerDescription")));
	if (UPBBumperTriggerDataAsset* TriggerDataAsset = SetupTriggerDataAsset(RowName))
	{
		NewRow.TriggerDataAsset = TSoftObjectPtr<UPBBumperTriggerDataAsset>(TriggerDataAsset);
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBumperTriggerDataAsset* UPBBumperTriggerTableParser::SetupTriggerDataAsset(const FName RowName) const
{
	UPBBumperTriggerDataAsset* TriggerDataAsset =
		GetOrCreateDataAsset<UPBBumperTriggerDataAsset>(DataAssetPreset, RowName, TEXT("Trigger"));
	if (!IsValid(TriggerDataAsset))
	{
		return nullptr;
	}

	TriggerDataAsset->TriggerClass =
		FindBlueprintClass<APBBumperTriggerActorBase>(TriggerClassPreset, RowName);
	(void)TriggerDataAsset->MarkPackageDirty();

	return TriggerDataAsset;
}
