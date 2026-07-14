#include "Synergy/PBSynergyEffectModifierTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyEffectModifierRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBSynergyEffectModifierTableParser::GetParserName() const
{
	return TEXT("SynergyEffectModifier");
}

UScriptStruct* UPBSynergyEffectModifierTableParser::GetRowStruct() const
{
	return FPBSynergyEffectModifierRow::StaticStruct();
}

bool UPBSynergyEffectModifierTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBSynergyEffectModifierRow NewRow;
	NewRow.SynergyEffectId = ParseNameValue(RowData.FindRef(TEXT("SynergyEffectId")));
	NewRow.ModifyType = ParseEnumValue(RowData.FindRef(TEXT("ModifyType")), EPBStatusEffectModifyType::Add);
	NewRow.Value = ParseFloatValue(RowData.FindRef(TEXT("Value")), 0.0f);
	NewRow.bScaleWithStack = ParseBoolValue(RowData.FindRef(TEXT("bScaleWithStack")), false);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
