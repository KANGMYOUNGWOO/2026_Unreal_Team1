// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"

using namespace PBSheetParserUtils;

UPBBumperEffectTableParser::UPBBumperEffectTableParser()
{
	EffectClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Effect");
	EffectClassPreset.NameFormat = TEXT("BP_{0}");
	BumperDataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper");
	BumperDataAssetPreset.NameFormat = TEXT("DA_Bumper_{0}");
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
	NewRow.ExecutionPolicy = ParseEnumValue(
		RowData.FindRef(TEXT("ExecutionPolicy")),
		EPBBumperEffectExecutionPolicy::Immediate);
	NewRow.Power = ParseFloatValue(RowData.FindRef(TEXT("Power")), 0.0f);
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));

	TargetTable->AddRow(RowName, NewRow);
	UpdateLinkedBumperDataAssets(RowName);
	return true;
}

void UPBBumperEffectTableParser::UpdateLinkedBumperDataAssets(const FName EffectId) const
{
	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	if (!IsValid(BumperTable) || EffectId.IsNone())
	{
		return;
	}

	const TSoftClassPtr<UPBBumperEffectBase> EffectClass =
		FindBlueprintClass<UPBBumperEffectBase>(EffectClassPreset, EffectId);
	for (const TPair<FName, uint8*>& RowPair : BumperTable->GetRowMap())
	{
		const FPBBumperTableRow* BumperRow = reinterpret_cast<FPBBumperTableRow*>(RowPair.Value);
		if (!BumperRow || BumperRow->EffectID != EffectId)
		{
			continue;
		}

		UPBBumperDataAsset* BumperDataAsset =
			FindObject<UPBBumperDataAsset>(BumperDataAssetPreset, RowPair.Key).LoadSynchronous();
		if (IsValid(BumperDataAsset))
		{
			BumperDataAsset->EffectClass = EffectClass;
			(void)BumperDataAsset->MarkPackageDirty();
		}
	}
}
