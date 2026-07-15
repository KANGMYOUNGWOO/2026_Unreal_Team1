#include "Skill/PBSkillTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

using namespace PBSheetParserUtils;

UPBSkillTableParser::UPBSkillTableParser()
{
	SkillActorClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Ball/Skill");
	SkillActorClassPreset.NameFormat = TEXT("BP_Skill_{0}");
	BallDataAssetPreset.FolderPath.Path = TEXT("/Game/Blueprints/Ball/DataAsset");
	BallDataAssetPreset.NameFormat = TEXT("DA_{0}");
}

const TCHAR* UPBSkillTableParser::GetParserName() const
{
	return TEXT("Skill");
}

UScriptStruct* UPBSkillTableParser::GetRowStruct() const
{
	return FPBBallSkillTableRow::StaticStruct();
}

bool UPBSkillTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBBallSkillTableRow NewRow;
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));
	NewRow.DamageMultiplier = FMath::Max(
		ParseFloatValue(RowData.FindRef(TEXT("Power")), 1.0f),
		0.0f);
	NewRow.Duration = FMath::Max(
		ParseFloatValue(RowData.FindRef(TEXT("Duration")), 2.0f),
		0.01f);
	NewRow.Value = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("Value")), 1),
		1);

	TargetTable->AddRow(RowName, NewRow);
	UpdateLinkedBallDataAssets(RowName);
	return true;
}

void UPBSkillTableParser::UpdateLinkedBallDataAssets(const FName SkillId) const
{
	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BallTable = Settings ? Settings->BallTable.LoadSynchronous() : nullptr;
	if (!IsValid(BallTable) || SkillId.IsNone())
	{
		return;
	}

	const TSoftClassPtr<APBBallSkillActorBase> SkillActorClass =
		FindBlueprintClass<APBBallSkillActorBase>(SkillActorClassPreset, SkillId);

	for (const TPair<FName, uint8*>& RowPair : BallTable->GetRowMap())
	{
		const FPBBallTableRow* BallRow = reinterpret_cast<FPBBallTableRow*>(RowPair.Value);
		if (!BallRow || !BallRow->DefaultSkillIds.Contains(SkillId))
		{
			continue;
		}

		UPBBallDataAsset* BallDataAsset =
			GetOrCreateDataAsset<UPBBallDataAsset>(BallDataAssetPreset, RowPair.Key, TEXT("Skill"));
		if (IsValid(BallDataAsset))
		{
			BallDataAsset->SkillActorClass = SkillActorClass;
			(void)BallDataAsset->MarkPackageDirty();
		}
	}
}
