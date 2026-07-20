#include "Effect/PBEffectSetTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectSetRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBEffectSetTableParser::GetParserName() const
{
	return TEXT("EffectSet");
}

UScriptStruct* UPBEffectSetTableParser::GetRowStruct() const
{
	return FPBEffectSetRow::StaticStruct();
}

bool UPBEffectSetTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBEffectSetRow NewRow;
	NewRow.EffectSetId = ParseNameValue(RowData.FindRef(TEXT("EffectSetId")));
	NewRow.EffectId = ParseNameValue(RowData.FindRef(TEXT("EffectId")));
	NewRow.Order = ParseIntValue(RowData.FindRef(TEXT("Order")), 0);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
