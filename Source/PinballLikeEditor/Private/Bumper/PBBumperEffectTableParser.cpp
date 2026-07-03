// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperEffectTableParser.h"

#include "Bumper/PBSheetParserUtils.h"
#include "PathDataLoadHelper.h"
#include "PinBallLike/DataAsset/Bumper/PBBumperEffectDataAsset.h"
#include "PinBallLike/Struct/Bumper/PBBumperEffectRow.h"

using namespace PBSheetParserUtils;

UPBBumperEffectTableParser::UPBBumperEffectTableParser()
{
	DataAssetFolderPath.Path = TEXT("/Game/DataAsset/Bumper/Effect");
}

void UPBBumperEffectTableParser::OnParseComplete()
{
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Effect] OnParseComplete. RowCount=%d"), GetRowCount());
	LogHeaders(TEXT("Effect"), GetHeaders());

	if (!ValidateTargetTable(TargetTable, FPBBumperEffectRow::StaticStruct()))
	{
		LogTargetTableError(TEXT("Effect"), TargetTable, FPBBumperEffectRow::StaticStruct());
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
			UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Effect] Skip row %d because RowName is empty."), RowIndex);
			continue;
		}

		const FName RowName(*TrimCell(RowNameString));
		FPBBumperEffectRow NewRow;
		NewRow.EffectType = ParseEnumValue(RowData.FindRef(TEXT("EffectType")), EPBBumperEffectType::Instant);
		NewRow.Power = ParseFloatValue(RowData.FindRef(TEXT("Power")), 0.0f);
		NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));
		if (UPBBumperEffectDataAsset* EffectDataAsset = SetupEffectDataAsset(RowName))
		{
			NewRow.EffectDataAsset = TSoftObjectPtr<UPBBumperEffectDataAsset>(EffectDataAsset);
		}

		TargetTable->AddRow(RowName, NewRow);
		++AddedRowCount;
	}

	MarkTargetTableDirty(TargetTable);
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Effect] Added %d rows to %s."),
		AddedRowCount,
		*GetNameSafe(TargetTable));
}

UPBBumperEffectDataAsset* UPBBumperEffectTableParser::SetupEffectDataAsset(const FName RowName) const
{
	const FString RowNameString = RowName.ToString();
	if (IsUnsetValue(RowNameString)
		|| IsUnsetValue(DataAssetFolderPath.Path)
		|| IsUnsetValue(AssetNameFormat))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Effect] Skip DataAsset setup because asset settings are empty. RowName=%s"),
			*RowNameString);
		return nullptr;
	}

	const FString AssetName = MakeGeneratedAssetName(AssetNameFormat, RowName);
	UPBBumperEffectDataAsset* EffectDataAsset =
		UPathDataLoadHelper::GetOrCreateAsset<UPBBumperEffectDataAsset>(DataAssetFolderPath.Path, AssetName);
	if (!IsValid(EffectDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperSheet][Effect] Failed to get or create DataAsset. RowName=%s AssetName=%s"),
			*RowNameString,
			*AssetName);
		return nullptr;
	}

	const FString EffectClassPath = FindBlueprintClassReferencePath(
		EffectClassFolderPath.Path,
		EffectClassNameFormat,
		RowName);
	if (!EffectClassPath.IsEmpty())
	{
		// 지정 폴더에서 찾은 BP 클래스를 EffectDataAsset에 연결함
		EffectDataAsset->EffectClass = TSoftClassPtr<UPBBumperEffectBase>(FSoftObjectPath(EffectClassPath));
	}

	// PrimaryAssetId가 에셋명이 아닌 시트 RowName을 사용하도록 저장함
	EffectDataAsset->RowName = RowName;
	(void)EffectDataAsset->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][Effect] Linked DataAsset. RowName=%s Asset=%s PrimaryAssetId=%s"),
		*RowNameString,
		*GetNameSafe(EffectDataAsset),
		*EffectDataAsset->GetPrimaryAssetId().ToString());

	return EffectDataAsset;
}
