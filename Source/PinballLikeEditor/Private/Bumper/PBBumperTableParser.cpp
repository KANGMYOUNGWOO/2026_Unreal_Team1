// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperTableParser.h"

#include "Bumper/PBSheetParserUtils.h"
#include "PathDataLoadHelper.h"
#include "PinBallLike/DataAsset/Bumper/PBBumperDataAsset.h"
#include "PinBallLike/Struct/Bumper/PBBumperTableRow.h"

using namespace PBSheetParserUtils;

UPBBumperTableParser::UPBBumperTableParser()
{
	DataAssetFolderPath.Path = TEXT("/Game/DataAsset/Bumper");
}

void UPBBumperTableParser::OnParseComplete()
{
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Bumper] OnParseComplete. RowCount=%d"), GetRowCount());
	LogHeaders(TEXT("Bumper"), GetHeaders());

	if (!ValidateTargetTable(TargetTable, FPBBumperTableRow::StaticStruct()))
	{
		LogTargetTableError(TEXT("Bumper"), TargetTable, FPBBumperTableRow::StaticStruct());
		return;
	}

	ResetTargetTable(TargetTable);
	int32 AddedRowCount = 0;

	for (int32 RowIndex = 0; RowIndex < GetRowCount(); ++RowIndex)
	{
		TMap<FString, FString> RowData;
		if (!GetRowAt(RowIndex, RowData))
		{
			continue;
		}

		const FString RowNameString = RowData.FindRef(TEXT("RowName"));
		if (IsUnsetValue(RowNameString))
		{
			UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Bumper] Skip row %d because RowName is empty."), RowIndex);
			continue;
		}

		const FName RowName(*TrimCell(RowNameString));
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
		++AddedRowCount;
	}

	MarkTargetTableDirty(TargetTable);
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Bumper] Added %d rows to %s."),
		AddedRowCount,
		*GetNameSafe(TargetTable));
}

UPBBumperDataAsset* UPBBumperTableParser::SetupBumperDataAsset(const FName RowName) const
{
	const FString RowNameString = RowName.ToString();
	if (IsUnsetValue(RowNameString)
		|| IsUnsetValue(DataAssetFolderPath.Path)
		|| IsUnsetValue(AssetNameFormat))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Bumper] Skip DataAsset setup because asset settings are empty. RowName=%s"),
			*RowNameString);
		return nullptr;
	}

	const FString AssetName = MakeGeneratedAssetName(AssetNameFormat, RowName);
	UPBBumperDataAsset* BumperDataAsset =
		UPathDataLoadHelper::GetOrCreateAsset<UPBBumperDataAsset>(DataAssetFolderPath.Path, AssetName);
	if (!IsValid(BumperDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Bumper] Failed to get or create DataAsset. RowName=%s AssetName=%s"),
			*RowNameString,
			*AssetName);
		return nullptr;
	}

	// PrimaryAssetId가 에셋명이 아닌 시트 RowName을 사용하도록 저장함
	BumperDataAsset->RowName = RowName;
	(void)BumperDataAsset->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Bumper] Linked DataAsset. RowName=%s Asset=%s PrimaryAssetId=%s"),
		*RowNameString,
		*GetNameSafe(BumperDataAsset),
		*BumperDataAsset->GetPrimaryAssetId().ToString());

	return BumperDataAsset;
}
