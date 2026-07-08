// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/PBBossPatternTableParser.h"

#include "PBSheetParserUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PinBallLike/Table/Boss/DataAsset/PBBossDataAsset.h"
#include "PinBallLike/Table/Boss/Struct/PBBossPatternTableRow.h"

using namespace PBSheetParserUtils;

namespace
{
bool ParseBoolCell(const FString& Value, const bool IsDefaultValue)
{
	const FString TrimmedValue = TrimCell(Value);
	if (IsUnsetValue(TrimmedValue))
	{
		return IsDefaultValue;
	}

	return TrimmedValue.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| TrimmedValue.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| TrimmedValue.Equals(TEXT("yes"), ESearchCase::IgnoreCase);
}

TSoftClassPtr<UPBBossPatternBase> FindPatternClassRecursive(const FName PatternClassId)
{
	if (PatternClassId.IsNone())
	{
		return nullptr;
	}

	const FString PatternClassIdString = PatternClassId.ToString();
	TSet<FName> CandidateNames;
	CandidateNames.Add(PatternClassId);
	CandidateNames.Add(FName(*FString::Printf(TEXT("BP_%s"), *PatternClassIdString)));
	CandidateNames.Add(FName(*FString::Printf(TEXT("Boss%s"), *PatternClassIdString)));
	CandidateNames.Add(FName(*FString::Printf(TEXT("BP_Boss%s"), *PatternClassIdString)));

	if (PatternClassIdString.Contains(TEXT("ProjectilePattern")))
	{
		const FString ProjectilePatternAlias = PatternClassIdString.Replace(TEXT("ProjectilePattern"), TEXT("PatternProjectile"));
		CandidateNames.Add(FName(*ProjectilePatternAlias));
		CandidateNames.Add(FName(*FString::Printf(TEXT("BP_%s"), *ProjectilePatternAlias)));
		CandidateNames.Add(FName(*FString::Printf(TEXT("Boss%s"), *ProjectilePatternAlias)));
		CandidateNames.Add(FName(*FString::Printf(TEXT("BP_Boss%s"), *ProjectilePatternAlias)));
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game/Blueprints/Boss/Pattern"));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDatas;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDatas);
	for (const FAssetData& AssetData : AssetDatas)
	{
		if (!CandidateNames.Contains(AssetData.AssetName))
		{
			continue;
		}

		const FString ClassPath = FString::Printf(TEXT("%s_C"), *AssetData.GetSoftObjectPath().ToString());
		return TSoftClassPtr<UPBBossPatternBase>(FSoftObjectPath(ClassPath));
	}

	return nullptr;
}
}

UPBBossPatternTableParser::UPBBossPatternTableParser()
{
	BossDataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Boss");
	BossDataAssetPreset.NameFormat = TEXT("DA_Boss_{0}");

	FPBSheetAssetPathPreset ChargePatternPreset;
	ChargePatternPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/Pattern/Charge");
	ChargePatternPreset.NameFormat = TEXT("BP_{0}");
	PatternClassPresets.Add(ChargePatternPreset);

	FPBSheetAssetPathPreset ProjectilePatternPreset;
	ProjectilePatternPreset.FolderPath.Path = TEXT("/Game/Blueprints/Boss/Pattern/projectile");
	ProjectilePatternPreset.NameFormat = TEXT("BP_{0}");
	PatternClassPresets.Add(ProjectilePatternPreset);
}

const TCHAR* UPBBossPatternTableParser::GetParserName() const
{
	return TEXT("BossPattern");
}

UScriptStruct* UPBBossPatternTableParser::GetRowStruct() const
{
	return FPBBossPatternTableRow::StaticStruct();
}

void UPBBossPatternTableParser::OnParseComplete()
{
	ClearedBossRowNames.Reset();
	Super::OnParseComplete();
}

bool UPBBossPatternTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBossPatternTableRow NewRow;
	NewRow.BossRowName = FName(*TrimCell(RowData.FindRef(TEXT("BossRowName"))));
	NewRow.PatternClassID = FName(*TrimCell(RowData.FindRef(TEXT("PatternClassID"))));
	NewRow.PatternPhaseType = ParseEnumValue(
		RowData.FindRef(TEXT("PatternPhaseType")),
		EPBBossPatternPhaseType::Normal);
	NewRow.PatternName = FName(*TrimCell(RowData.FindRef(TEXT("PatternName"))));
	NewRow.CooldownSeconds = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("CooldownSeconds")), 3.0f), 0.0f);
	NewRow.IsEnabled = ParseBoolCell(RowData.FindRef(TEXT("IsEnabled")), true);

	TargetTable->AddRow(RowName, NewRow);

	ClearBossPatternData(NewRow.BossRowName);
	if (NewRow.IsEnabled)
	{
		UpdateLinkedBossDataAsset(
			NewRow.BossRowName,
			NewRow.PatternClassID,
			NewRow.PatternPhaseType,
			NewRow.PatternName,
			NewRow.CooldownSeconds,
			NewRow.IsEnabled);
	}

	return true;
}

void UPBBossPatternTableParser::UpdateLinkedBossDataAsset(
	const FName BossRowName,
	const FName PatternClassId,
	const EPBBossPatternPhaseType PatternPhaseType,
	const FName PatternName,
	const float CooldownSeconds,
	const bool IsEnabled)
{
	if (BossRowName.IsNone() || PatternClassId.IsNone())
	{
		return;
	}

	UPBBossDataAsset* BossDataAsset =
		GetOrCreateDataAsset<UPBBossDataAsset>(BossDataAssetPreset, BossRowName, TEXT("BossPattern"));
	if (!IsValid(BossDataAsset))
	{
		return;
	}

	const TSoftClassPtr<UPBBossPatternBase> PatternClass = FindPatternClass(PatternClassId);
	if (PatternClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Sheet][BossPattern] Pattern class not found. BossRowName=%s PatternClassID=%s"),
			*BossRowName.ToString(),
			*PatternClassId.ToString());
		return;
	}

	FPBBossPatternData PatternData;
	PatternData.PatternClass = PatternClass;
	PatternData.PatternName = PatternName;
	PatternData.CooldownSeconds = CooldownSeconds;
	PatternData.IsEnabled = IsEnabled;

	switch (PatternPhaseType)
	{
	case EPBBossPatternPhaseType::Normal:
		BossDataAsset->PatternDatas.Add(PatternData);
		break;
	case EPBBossPatternPhaseType::Enraged:
		BossDataAsset->EnragedPatternDatas.Add(PatternData);
		break;
	case EPBBossPatternPhaseType::EnragedEntry:
		BossDataAsset->EnragedEntryPatternDatas.Add(PatternData);
		break;
	default:
		break;
	}

	(void)BossDataAsset->MarkPackageDirty();
}

void UPBBossPatternTableParser::ClearBossPatternData(const FName BossRowName)
{
	if (BossRowName.IsNone() || ClearedBossRowNames.Contains(BossRowName))
	{
		return;
	}

	ClearedBossRowNames.Add(BossRowName);

	UPBBossDataAsset* BossDataAsset =
		GetOrCreateDataAsset<UPBBossDataAsset>(BossDataAssetPreset, BossRowName, TEXT("BossPattern"));
	if (!IsValid(BossDataAsset))
	{
		return;
	}

	BossDataAsset->PatternDatas.Reset();
	BossDataAsset->EnragedPatternDatas.Reset();
	BossDataAsset->EnragedEntryPatternDatas.Reset();
	(void)BossDataAsset->MarkPackageDirty();
}

TSoftClassPtr<UPBBossPatternBase> UPBBossPatternTableParser::FindPatternClass(const FName PatternClassId) const
{
	for (const FPBSheetAssetPathPreset& PatternClassPreset : PatternClassPresets)
	{
		if (!PatternClassPreset.IsValid())
		{
			continue;
		}

		TSoftClassPtr<UPBBossPatternBase> PatternClass =
			FindBlueprintClass<UPBBossPatternBase>(PatternClassPreset, PatternClassId);
		if (!PatternClass.IsNull())
		{
			return PatternClass;
		}
	}

	return FindPatternClassRecursive(PatternClassId);
}
