#include "Synergy/PBSynergyEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyEffectRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBSynergyEffectTableParser::GetParserName() const
{
	return TEXT("SynergyEffect");
}

UScriptStruct* UPBSynergyEffectTableParser::GetRowStruct() const
{
	return FPBSynergyEffectRow::StaticStruct();
}

bool UPBSynergyEffectTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBSynergyEffectRow NewRow;
	NewRow.StackType = ParseEnumValue(RowData.FindRef(TEXT("StackType")), EPBStatusEffectStackType::Replace);
	NewRow.DurationPolicy = ParseEnumValue(RowData.FindRef(TEXT("DurationPolicy")), EPBStatusEffectDurationPolicy::Permanent);
	NewRow.DurationValue = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("DurationValue")), 0.0f), 0.0f);
	NewRow.Interval = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("Interval")), 0.0f), 0.0f);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
