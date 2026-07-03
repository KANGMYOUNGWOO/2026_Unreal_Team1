// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperTriggerTableParser.h"

#include "Bumper/PBSheetParserUtils.h"
#include "PathDataLoadHelper.h"
#include "PinBallLike/DataAsset/Bumper/PBBumperTriggerDataAsset.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerRow.h"

using namespace PBSheetParserUtils;

UPBBumperTriggerTableParser::UPBBumperTriggerTableParser()
{
	DataAssetFolderPath.Path = TEXT("/Game/DataAsset/Bumper/Trigger");
}

void UPBBumperTriggerTableParser::OnParseComplete()
{
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Trigger] OnParseComplete. RowCount=%d"), GetRowCount());
	LogHeaders(TEXT("Trigger"), GetHeaders());

	if (!ValidateTargetTable(TargetTable, FPBBumperTriggerRow::StaticStruct()))
	{
		LogTargetTableError(TEXT("Trigger"), TargetTable, FPBBumperTriggerRow::StaticStruct());
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
			UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Trigger] Skip row %d because RowName is empty."), RowIndex);
			continue;
		}

		const FName RowName(*TrimCell(RowNameString));
		FPBBumperTriggerRow NewRow;
		NewRow.TriggerType = ParseEnumValue(RowData.FindRef(TEXT("TriggerType")), EPBBumperTriggerType::HitCount);
		NewRow.PositionIds = ParsePositionIds(RowData.FindRef(TEXT("PositionIds")));
		NewRow.TriggerDescription = FText::FromString(RowData.FindRef(TEXT("TriggerDescription")));
		if (UPBBumperTriggerDataAsset* TriggerDataAsset = SetupTriggerDataAsset(RowName))
		{
			NewRow.TriggerDataAsset = TSoftObjectPtr<UPBBumperTriggerDataAsset>(TriggerDataAsset);
		}

		TargetTable->AddRow(RowName, NewRow);
		++AddedRowCount;
	}

	MarkTargetTableDirty(TargetTable);
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Trigger] Added %d rows to %s."),
		AddedRowCount,
		*GetNameSafe(TargetTable));
}

UPBBumperTriggerDataAsset* UPBBumperTriggerTableParser::SetupTriggerDataAsset(const FName RowName) const
{
	const FString RowNameString = RowName.ToString();
	if (IsUnsetValue(RowNameString)
		|| IsUnsetValue(DataAssetFolderPath.Path)
		|| IsUnsetValue(AssetNameFormat))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Trigger] Skip DataAsset setup because asset settings are empty. RowName=%s"),
			*RowNameString);
		return nullptr;
	}

	const FString AssetName = MakeGeneratedAssetName(AssetNameFormat, RowName);
	UPBBumperTriggerDataAsset* TriggerDataAsset =
		UPathDataLoadHelper::GetOrCreateAsset<UPBBumperTriggerDataAsset>(DataAssetFolderPath.Path, AssetName);
	if (!IsValid(TriggerDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Trigger] Failed to get or create DataAsset. RowName=%s AssetName=%s"),
			*RowNameString,
			*AssetName);
		return nullptr;
	}

	const FString TriggerClassPath = FindBlueprintClassReferencePath(
		TriggerClassFolderPath.Path,
		TriggerClassNameFormat,
		RowName);
	if (!TriggerClassPath.IsEmpty())
	{
		// 지정 폴더에서 찾은 BP 클래스를 TriggerDataAsset에 연결함
		TriggerDataAsset->TriggerClass = TSoftClassPtr<APBBumperTriggerActorBase>(FSoftObjectPath(TriggerClassPath));
	}

	// PrimaryAssetId가 에셋명이 아닌 시트 RowName을 사용하도록 저장함
	TriggerDataAsset->RowName = RowName;
	(void)TriggerDataAsset->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Trigger] Linked DataAsset. RowName=%s Asset=%s PrimaryAssetId=%s"),
		*RowNameString,
		*GetNameSafe(TriggerDataAsset),
		*TriggerDataAsset->GetPrimaryAssetId().ToString());

	return TriggerDataAsset;
}
