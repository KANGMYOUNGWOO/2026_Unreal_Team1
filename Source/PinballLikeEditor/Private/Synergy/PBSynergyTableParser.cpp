#include "Synergy/PBSynergyTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"

using namespace PBSheetParserUtils;

UPBSynergyTableParser::UPBSynergyTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Synergy");
	DataAssetPreset.NameFormat = TEXT("DA_Synergy_{0}");
	IconPreset.FolderPath.Path = TEXT("/Game/Resources/Synergy");
	IconPreset.NameFormat = TEXT("Icon_{0}");
}

const TCHAR* UPBSynergyTableParser::GetParserName() const
{
	return TEXT("Synergy");
}

UScriptStruct* UPBSynergyTableParser::GetRowStruct() const
{
	return FPBSynergyTableRow::StaticStruct();
}

bool UPBSynergyTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBSynergyTableRow NewRow;
	NewRow.DisplayName = FText::FromString(TrimCell(RowData.FindRef(TEXT("DisplayName"))));
	NewRow.DescriptionKey = ParseNameValue(RowData.FindRef(TEXT("Description")));
	NewRow.SynergyKind = ParseEnumValue(RowData.FindRef(TEXT("SynergyKind")), EPBSynergyKind::Race);
	NewRow.RuleType = ParseEnumValue(RowData.FindRef(TEXT("RuleType")), EPBSynergyRuleType::Stat);
	NewRow.SortOrder = FMath::RoundToInt(ParseFloatValue(RowData.FindRef(TEXT("SortOrder")), 0.0f));

	(void)SetupSynergyDataAsset(RowName, RowData);
	
	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBSynergyDataAsset* UPBSynergyTableParser::SetupSynergyDataAsset(FName RowName,
	const TMap<FString, FString>& RowData) const
{
	UPBSynergyDataAsset* SynergyDataAsset =
	GetOrCreateDataAsset<UPBSynergyDataAsset>(DataAssetPreset, RowName, TEXT("Synergy"));
	if (!IsValid(SynergyDataAsset))
	{
		return nullptr;
	}

	if (IconPreset.IsValid())
	{
		const FString IconIdString = TrimCell(RowData.FindRef(TEXT("Icon")));
		const FName IconId = IsUnsetValue(IconIdString) ? RowName : FName(*IconIdString);
		SynergyDataAsset->Icon = FindObject<UTexture2D>(IconPreset, IconId);
	}

	(void)SynergyDataAsset->MarkPackageDirty();
	return SynergyDataAsset;
}
