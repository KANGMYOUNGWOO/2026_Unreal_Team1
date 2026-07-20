// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/PBBossTableParser.h"

#include "PBSheetParserUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Texture2D.h"
#include "Modules/ModuleManager.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossUILayerWidget.h"
#include "PinBallLike/Table/Boss/DataAsset/PBBossDataAsset.h"
#include "PinBallLike/Table/Boss/Struct/PBBossTableRow.h"

using namespace PBSheetParserUtils;

FName UPBBossTableParser::GetClassIdOrRowName(const TMap<FString, FString>& RowData, const FString& ColumnName, const FName RowName) const
{
	const FString ClassId = TrimCell(RowData.FindRef(ColumnName));
	return IsUnsetValue(ClassId) ? RowName : FName(*ClassId);
}

UPBBossTableParser::UPBBossTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Boss");
	DataAssetPreset.NameFormat = TEXT("DA_Boss_{0}");
	IconPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/BossAsset");
	IconPreset.NameFormat = TEXT("{0}");

	BossUILayerClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/UI");
	BossUILayerClassPreset.NameFormat = TEXT("WBP_{0}");
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
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.MaxHP = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxHP")), 100), 1);
	NewRow.MaxGroggyGauge = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxGroggyGauge")), 100), 1);
	NewRow.GroggyDurationSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("GroggyDurationSeconds")), 3.0f), 0.1f);
	NewRow.EnrageHPRatioPercent = FMath::Clamp(ParseIntValue(RowData.FindRef(TEXT("EnrageHPRatioPercent")), 40), 0, 100);
	NewRow.DefaultHitPointName = FName(*TrimCell(RowData.FindRef(TEXT("DefaultHitPointName"))));
	NewRow.DamageCooldownSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("DamageCooldownSeconds")), 0.25f), 0.0f);
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

	const FName BossUILayerClassId = GetClassIdOrRowName(RowData, TEXT("BossUILayerClassID"), NAME_None);
	const FName EnrageCameraShakeClassId = GetClassIdOrRowName(RowData, TEXT("EnrageCameraShakeClassID"), NAME_None);
	const FString IconIdString = TrimCell(RowData.FindRef(TEXT("Icon")));

	BossDataAsset->BossName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	if (IconPreset.IsValid() && !IsUnsetValue(IconIdString))
	{
		BossDataAsset->BossIntroImage = FindObject<UTexture2D>(IconPreset, FName(*IconIdString));
	}
	else
	{
		BossDataAsset->BossIntroImage.Reset();
	}
	BossDataAsset->MaxHP = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxHP")), 100), 1);
	BossDataAsset->MaxGroggyGauge = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("MaxGroggyGauge")), 100), 1);
	BossDataAsset->GroggyDurationSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("GroggyDurationSeconds")), 3.0f), 0.1f);
	BossDataAsset->EnrageHPRatioPercent = FMath::Clamp(ParseIntValue(RowData.FindRef(TEXT("EnrageHPRatioPercent")), 40), 0, 100);
	BossDataAsset->DefaultHitPointName = FName(*TrimCell(RowData.FindRef(TEXT("DefaultHitPointName"))));
	BossDataAsset->DamageCooldownSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("DamageCooldownSeconds")), 0.25f), 0.0f);
	BossDataAsset->PatternCheckIntervalSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("PatternCheckIntervalSeconds")), 0.25f), 0.1f);
	if (BossDataAsset->DefaultHitPointName.IsNone())
	{
		BossDataAsset->DefaultHitPointName = TEXT("Normal");
	}

	BossDataAsset->BossClass = FindBossClass(RowName);

	if (BossUILayerClassPreset.IsValid() && !BossUILayerClassId.IsNone())
	{
		BossDataAsset->BossUILayerClass =
			FindBlueprintClass<UPBBossUILayerWidget>(BossUILayerClassPreset, BossUILayerClassId);
	}
	else
	{
		BossDataAsset->BossUILayerClass.Reset();
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
	if (BossClassId.IsNone())
	{
		return nullptr;
	}

	const FName BossAssetName(*FString::Printf(TEXT("BP_%s"), *BossClassId.ToString()));
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game/Blueprints/Boss"));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDatas;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDatas);
	for (const FAssetData& AssetData : AssetDatas)
	{
		if (AssetData.AssetName != BossAssetName)
		{
			continue;
		}

		const FString ClassPath = FString::Printf(TEXT("%s_C"), *AssetData.GetSoftObjectPath().ToString());
		return TSoftClassPtr<APBBossBase>(FSoftObjectPath(ClassPath));
	}

	return nullptr;
}
