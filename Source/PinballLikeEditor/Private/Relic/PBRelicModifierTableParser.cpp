
#include "PinballLikeEditor/Public/Relic/PBRelicModifierTableParser.h"
#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicModifierRow.h"


using namespace PBSheetParserUtils;

const TCHAR* UPBRelicModifierTableParser::GetParserName() const
{
	return TEXT("RelicModifier");
}

UScriptStruct* UPBRelicModifierTableParser::GetRowStruct() const
{
	return FPBRelicModifierRow::StaticStruct();
}

bool UPBRelicModifierTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBRelicModifierRow NewRow;

	NewRow.RelicId =
		FName(*TrimCell(
			RowData.FindRef(TEXT("RelicId"))));

	NewRow.TargetType =
		ParseEnumValue(
			RowData.FindRef(TEXT("TargetType")),
			EPBRelicTargetType::None);

	NewRow.TargetStat =
		FName(*TrimCell(
			RowData.FindRef(TEXT("TargetStat"))));

	NewRow.ModifyType =
		ParseEnumValue(
			RowData.FindRef(TEXT("ModifyType")),
			EPBRelicModifyType::Add);

	NewRow.Value =
		ParseFloatValue(
			RowData.FindRef(TEXT("Value")),
			0.0f);

	TargetTable->AddRow(RowName, NewRow);

	return true;
}