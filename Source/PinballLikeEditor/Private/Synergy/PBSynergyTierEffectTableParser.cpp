#include "Synergy/PBSynergyTierEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierEffectRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBSynergyTierEffectTableParser::GetParserName() const
{
	return TEXT("SynergyTierEffect");
}

UScriptStruct* UPBSynergyTierEffectTableParser::GetRowStruct() const
{
	return FPBSynergyTierEffectRow::StaticStruct();
}

bool UPBSynergyTierEffectTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBSynergyTierEffectRow NewRow;
	NewRow.SynergyTierId = ParseNameValue(RowData.FindRef(TEXT("SynergyTierId")));
	NewRow.SynergyEffectId = ParseNameValue(RowData.FindRef(TEXT("SynergyEffectId")));
	NewRow.ApplyOrder = FMath::RoundToInt(ParseFloatValue(RowData.FindRef(TEXT("ApplyOrder")), 0.0f));

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
