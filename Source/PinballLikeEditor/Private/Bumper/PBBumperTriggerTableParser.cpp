// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperTriggerTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"

using namespace PBSheetParserUtils;

UPBBumperTriggerTableParser::UPBBumperTriggerTableParser()
{
	TriggerClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Trigger");
	TriggerClassPreset.NameFormat = TEXT("BP_{0}");
	BumperDataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper");
	BumperDataAssetPreset.NameFormat = TEXT("DA_Bumper_{0}");
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

	TargetTable->AddRow(RowName, NewRow);
	UpdateLinkedBumperDataAssets(RowName);
	return true;
}

void UPBBumperTriggerTableParser::UpdateLinkedBumperDataAssets(const FName TriggerId) const
{
	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	if (!IsValid(BumperTable) || TriggerId.IsNone())
	{
		return;
	}

	const TSoftClassPtr<APBBumperTriggerActorBase> TriggerClass =
		FindBlueprintClass<APBBumperTriggerActorBase>(TriggerClassPreset, TriggerId);
	for (const TPair<FName, uint8*>& RowPair : BumperTable->GetRowMap())
	{
		const FPBBumperTableRow* BumperRow = reinterpret_cast<FPBBumperTableRow*>(RowPair.Value);
		if (!BumperRow || BumperRow->TriggerID != TriggerId)
		{
			continue;
		}

		UPBBumperDataAsset* BumperDataAsset =
			GetOrCreateDataAsset<UPBBumperDataAsset>(BumperDataAssetPreset, RowPair.Key, TEXT("Trigger"));
		if (IsValid(BumperDataAsset))
		{
			BumperDataAsset->TriggerClass = TriggerClass;
			(void)BumperDataAsset->MarkPackageDirty();
		}
	}
}
