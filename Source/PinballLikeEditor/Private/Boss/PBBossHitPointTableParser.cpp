// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/PBBossHitPointTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Boss/DataAsset/PBBossDataAsset.h"
#include "PinBallLike/Table/Boss/Struct/PBBossHitPointData.h"
#include "PinBallLike/Table/Boss/Struct/PBBossHitPointTableRow.h"

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
}

UPBBossHitPointTableParser::UPBBossHitPointTableParser()
{
	BossDataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Boss");
	BossDataAssetPreset.NameFormat = TEXT("DA_Boss_{0}");
}

const TCHAR* UPBBossHitPointTableParser::GetParserName() const
{
	return TEXT("BossHitPoint");
}

UScriptStruct* UPBBossHitPointTableParser::GetRowStruct() const
{
	return FPBBossHitPointTableRow::StaticStruct();
}

void UPBBossHitPointTableParser::OnParseComplete()
{
	ClearedBossRowNames.Reset();
	Super::OnParseComplete();
}

bool UPBBossHitPointTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBossHitPointTableRow NewRow;
	NewRow.BossRowName = FName(*TrimCell(RowData.FindRef(TEXT("BossRowName"))));
	NewRow.HitPointName = FName(*TrimCell(RowData.FindRef(TEXT("HitPointName"))));
	NewRow.HitPartType = ParseEnumValue(RowData.FindRef(TEXT("HitPartType")), EPBBossHitPartType::Body);
	NewRow.HPDamageMultiplierPercent = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("HPDamageMultiplierPercent")), 100),
		0);
	NewRow.GroggyMultiplierPercent = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("GroggyMultiplierPercent")), 100),
		0);
	NewRow.IsWeaknessPoint = ParseBoolCell(RowData.FindRef(TEXT("IsWeaknessPoint")), false);

	TargetTable->AddRow(RowName, NewRow);

	ClearBossHitPointData(NewRow.BossRowName);
	UpdateLinkedBossDataAsset(
		NewRow.BossRowName,
		NewRow.HitPointName,
		NewRow.HitPartType,
		NewRow.HPDamageMultiplierPercent,
		NewRow.GroggyMultiplierPercent,
		NewRow.IsWeaknessPoint);

	return true;
}

void UPBBossHitPointTableParser::ClearBossHitPointData(const FName BossRowName)
{
	if (BossRowName.IsNone() || ClearedBossRowNames.Contains(BossRowName))
	{
		return;
	}

	ClearedBossRowNames.Add(BossRowName);

	UPBBossDataAsset* BossDataAsset =
		GetOrCreateDataAsset<UPBBossDataAsset>(BossDataAssetPreset, BossRowName, TEXT("BossHitPoint"));
	if (!IsValid(BossDataAsset))
	{
		return;
	}

	BossDataAsset->HitPointDatas.Reset();
	(void)BossDataAsset->MarkPackageDirty();
}

void UPBBossHitPointTableParser::UpdateLinkedBossDataAsset(
	const FName BossRowName,
	const FName HitPointName,
	const EPBBossHitPartType HitPartType,
	const int32 HPDamageMultiplierPercent,
	const int32 GroggyMultiplierPercent,
	const bool IsWeaknessPoint)
{
	if (BossRowName.IsNone() || HitPointName.IsNone())
	{
		return;
	}

	UPBBossDataAsset* BossDataAsset =
		GetOrCreateDataAsset<UPBBossDataAsset>(BossDataAssetPreset, BossRowName, TEXT("BossHitPoint"));
	if (!IsValid(BossDataAsset))
	{
		return;
	}

	FPBBossHitPointData HitPointData;
	HitPointData.HitPointName = HitPointName;
	HitPointData.HitPartType = HitPartType;
	HitPointData.HPDamageMultiplierPercent = HPDamageMultiplierPercent;
	HitPointData.GroggyMultiplierPercent = GroggyMultiplierPercent;
	HitPointData.IsWeaknessPoint = IsWeaknessPoint;

	BossDataAsset->HitPointDatas.Add(HitPointData);
	(void)BossDataAsset->MarkPackageDirty();
}
