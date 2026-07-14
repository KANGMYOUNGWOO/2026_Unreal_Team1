#include "Synergy/PBSynergyEffectTriggerTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyEffectTriggerRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBSynergyEffectTriggerTableParser::GetParserName() const
{
	return TEXT("SynergyEffectTrigger");
}

UScriptStruct* UPBSynergyEffectTriggerTableParser::GetRowStruct() const
{
	return FPBSynergyEffectTriggerRow::StaticStruct();
}

bool UPBSynergyEffectTriggerTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBSynergyEffectTriggerRow NewRow;
	NewRow.SynergyEffectId = ParseNameValue(RowData.FindRef(TEXT("SynergyEffectId")));
	NewRow.Value = ParseFloatValue(RowData.FindRef(TEXT("Value")), 0.0f);
	NewRow.TargetEffectId = ParseNameValue(RowData.FindRef(TEXT("TargetEffectId")));
	NewRow.TargetTag = ParseNameValue(RowData.FindRef(TEXT("TargetTag")));
	NewRow.ProcChance = FMath::Clamp(ParseFloatValue(RowData.FindRef(TEXT("ProcChance")), 100.0f), 0.0f, 100.0f);
	NewRow.bScaleWithStack = ParseBoolValue(RowData.FindRef(TEXT("bScaleWithStack")), false);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
