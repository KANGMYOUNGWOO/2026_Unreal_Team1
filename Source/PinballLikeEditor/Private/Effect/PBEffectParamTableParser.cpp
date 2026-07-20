#include "Effect/PBEffectParamTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBEffectParamTableParser::GetParserName() const
{
	return TEXT("EffectParam");
}

UScriptStruct* UPBEffectParamTableParser::GetRowStruct() const
{
	return FPBEffectParamRow::StaticStruct();
}

bool UPBEffectParamTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBEffectParamRow NewRow;
	NewRow.EffectId = ParseNameValue(RowData.FindRef(TEXT("EffectId")));
	NewRow.ParamKey = ParseNameValue(RowData.FindRef(TEXT("ParamKey")));
	NewRow.ParamValue = TrimCell(RowData.FindRef(TEXT("ParamValue")));

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
