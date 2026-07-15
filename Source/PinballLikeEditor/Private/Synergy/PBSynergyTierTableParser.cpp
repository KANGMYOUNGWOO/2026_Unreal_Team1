#include "Synergy/PBSynergyTierTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBSynergyTierTableParser::GetParserName() const
{
	return TEXT("SynergyTier");
}

UScriptStruct* UPBSynergyTierTableParser::GetRowStruct() const
{
	return FPBSynergyTierRow::StaticStruct();
}

bool UPBSynergyTierTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBSynergyTierRow NewRow;
	NewRow.SynergyId = ParseNameValue(RowData.FindRef(TEXT("SynergyId")));
	NewRow.RequiredCount = FMath::Max(0, FMath::RoundToInt(ParseFloatValue(RowData.FindRef(TEXT("RequiredCount")), 0.0f)));
	NewRow.SynergyEffectId = ParseNameValue(RowData.FindRef(TEXT("SynergyEffectId")));
	NewRow.TierDescriptionKey = ParseNameValue(RowData.FindRef(TEXT("TierDescriptionKey")));

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
