#include "Skill/PBSkillTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
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
	SkillIconPreset.FolderPath.Path = TEXT("/Game/Resources/Skill");
	SkillIconPreset.NameFormat = TEXT("T_{0}");
	BallDataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Ball");
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
	NewRow.PowerValue = FMath::Max(
		ParseFloatValue(RowData.FindRef(TEXT("PowerValue")), 1.0f),
		0.0f);
	NewRow.LifeValue = FMath::Max(
		ParseFloatValue(RowData.FindRef(TEXT("LifeValue")), 2.0f),
		0.0f);
	NewRow.EffectValue = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("EffectValue")), 1),
		0);
	NewRow.GroggyValue = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("GroggyValue")), 0),
		0);

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
	const TSoftObjectPtr<UTexture2D> SkillIcon =
		FindObject<UTexture2D>(SkillIconPreset, SkillId);

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
			BallDataAsset->SkillIcon = SkillIcon;
			(void)BallDataAsset->MarkPackageDirty();
		}
	}
}
