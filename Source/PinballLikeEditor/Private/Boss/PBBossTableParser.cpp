// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/PBBossTableParser.h"

#include "PBSheetParserUtils.h"
#include "Camera/CameraShakeBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossStatusWidget.h"
#include "PinBallLike/Table/Boss/DataAsset/PBBossDataAsset.h"
#include "PinBallLike/Table/Boss/Struct/PBBossTableRow.h"

using namespace PBSheetParserUtils;

namespace
{
FName GetClassIdOrRowName(const TMap<FString, FString>& RowData, const FString& ColumnName, const FName RowName)
{
	const FString ClassId = TrimCell(RowData.FindRef(ColumnName));
	return IsUnsetValue(ClassId) ? RowName : FName(*ClassId);
}
}

UPBBossTableParser::UPBBossTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Boss");
	DataAssetPreset.NameFormat = TEXT("DA_Boss_{0}");

	FPBSheetAssetPathPreset BossRootClassPreset;
	BossRootClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss");
	BossRootClassPreset.NameFormat = TEXT("BP_{0}");
	BossClassPresets.Add(BossRootClassPreset);

	FPBSheetAssetPathPreset SnakeBossClassPreset;
	SnakeBossClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/Snake");
	SnakeBossClassPreset.NameFormat = TEXT("BP_{0}");
	BossClassPresets.Add(SnakeBossClassPreset);

	BossStatusWidgetClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/UI");
	BossStatusWidgetClassPreset.NameFormat = TEXT("WBP_{0}");
	EnrageCameraShakeClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/UI");
	EnrageCameraShakeClassPreset.NameFormat = TEXT("BP_{0}");
}

const TCHAR* UPBBossTableParser::GetParserName() const
{
	return TEXT("Boss");
}

UScriptStruct* UPBBossTableParser::GetRowStruct() const
{
	return FPBBossTableRow::StaticStruct();
}

bool UPBBossTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBossTableRow NewRow;
	NewRow.BossId = ParseIntValue(RowData.FindRef(TEXT("BossId")), 0);
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.BossMovementType = ParseEnumValue(RowData.FindRef(TEXT("BossMovementType")), EPBBossMovementType::Fixed);
	NewRow.MaxHP = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxHP")), 100), 1);
	NewRow.MaxGroggyGauge = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxGroggyGauge")), 100), 1);
	NewRow.GroggyDurationSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("GroggyDurationSeconds")), 3.0f), 0.1f);
	NewRow.EnrageHPRatioPercent = FMath::Clamp(ParseIntValue(RowData.FindRef(TEXT("EnrageHPRatioPercent")), 40), 0, 100);
	NewRow.DefaultHitPointName = FName(*TrimCell(RowData.FindRef(TEXT("DefaultHitPointName"))));
	NewRow.DamageCooldownSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("DamageCooldownSeconds")), 0.25f), 0.0f);
	NewRow.MinPatternIntervalSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("MinPatternIntervalSeconds")), 2.0f), 0.0f);
	NewRow.PatternCheckIntervalSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("PatternCheckIntervalSeconds")), 0.25f), 0.1f);

	if (NewRow.DefaultHitPointName.IsNone())
	{
		NewRow.DefaultHitPointName = TEXT("Normal");
	}

	if (UPBBossDataAsset* BossDataAsset = SetupBossDataAsset(RowName, RowData))
	{
		NewRow.BossDataAsset = TSoftObjectPtr<UPBBossDataAsset>(BossDataAsset);
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBossDataAsset* UPBBossTableParser::SetupBossDataAsset(
	const FName RowName,
	const TMap<FString, FString>& RowData) const
{
	UPBBossDataAsset* BossDataAsset =
		GetOrCreateDataAsset<UPBBossDataAsset>(DataAssetPreset, RowName, TEXT("Boss"));
	if (!IsValid(BossDataAsset))
	{
		return nullptr;
	}

	const FName BossClassId = GetClassIdOrRowName(RowData, TEXT("BossClassID"), RowName);
	const FName BossStatusWidgetClassId = GetClassIdOrRowName(RowData, TEXT("BossStatusWidgetClassID"), NAME_None);
	const FName EnrageCameraShakeClassId = GetClassIdOrRowName(RowData, TEXT("EnrageCameraShakeClassID"), NAME_None);

	BossDataAsset->BossName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	BossDataAsset->BossMovementType = ParseEnumValue(RowData.FindRef(TEXT("BossMovementType")), EPBBossMovementType::Fixed);
	BossDataAsset->MaxHP = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxHP")), 100), 1);
	BossDataAsset->MaxGroggyGauge = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxGroggyGauge")), 100), 1);
	BossDataAsset->GroggyDurationSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("GroggyDurationSeconds")), 3.0f), 0.1f);
	BossDataAsset->EnrageHPRatioPercent = FMath::Clamp(ParseIntValue(RowData.FindRef(TEXT("EnrageHPRatioPercent")), 40), 0, 100);
	BossDataAsset->DefaultHitPointName = FName(*TrimCell(RowData.FindRef(TEXT("DefaultHitPointName"))));
	BossDataAsset->DamageCooldownSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("DamageCooldownSeconds")), 0.25f), 0.0f);
	BossDataAsset->MinPatternIntervalSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("MinPatternIntervalSeconds")), 2.0f), 0.0f);
	BossDataAsset->PatternCheckIntervalSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("PatternCheckIntervalSeconds")), 0.25f), 0.1f);
	if (BossDataAsset->DefaultHitPointName.IsNone())
	{
		BossDataAsset->DefaultHitPointName = TEXT("Normal");
	}

	if (!BossClassId.IsNone())
	{
		BossDataAsset->BossClass = FindBossClass(BossClassId);
	}

	if (BossStatusWidgetClassPreset.IsValid() && !BossStatusWidgetClassId.IsNone())
	{
		BossDataAsset->BossStatusWidgetClass =
			FindBlueprintClass<UPBBossStatusWidget>(BossStatusWidgetClassPreset, BossStatusWidgetClassId);
	}

	if (EnrageCameraShakeClassPreset.IsValid() && !EnrageCameraShakeClassId.IsNone())
	{
		BossDataAsset->EnrageCameraShakeClass =
			FindBlueprintClass<UCameraShakeBase>(EnrageCameraShakeClassPreset, EnrageCameraShakeClassId);
	}

	(void)BossDataAsset->MarkPackageDirty();
	return BossDataAsset;
}

TSoftClassPtr<APBBossBase> UPBBossTableParser::FindBossClass(const FName BossClassId) const
{
	for (const FPBSheetAssetPathPreset& BossClassPreset : BossClassPresets)
	{
		if (!BossClassPreset.IsValid())
		{
			continue;
		}

		TSoftClassPtr<APBBossBase> BossClass = FindBlueprintClass<APBBossBase>(BossClassPreset, BossClassId);
		if (!BossClass.IsNull())
		{
			return BossClass;
		}
	}

	return nullptr;
}
