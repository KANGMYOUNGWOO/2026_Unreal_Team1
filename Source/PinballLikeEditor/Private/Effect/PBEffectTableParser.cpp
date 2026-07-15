#include "Effect/PBEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBEffectTableParser::GetParserName() const
{
	return TEXT("Effect");
}

UScriptStruct* UPBEffectTableParser::GetRowStruct() const
{
	return FPBEffectTableRow::StaticStruct();
}

bool UPBEffectTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBEffectTableRow NewRow;
	NewRow.EffectType = ParseNameValue(RowData.FindRef(TEXT("EffectType")));
	NewRow.TargetType = ParseNameValue(RowData.FindRef(TEXT("TargetType")));
	NewRow.TargetFilter = ParseNameValue(RowData.FindRef(TEXT("TargetFilter")));
	NewRow.Description = ParseNameValue(RowData.FindRef(TEXT("Description")));
	NewRow.LinkedStatusEffectId = ParseNameValue(RowData.FindRef(TEXT("LinkedStatusEffectId")));

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
