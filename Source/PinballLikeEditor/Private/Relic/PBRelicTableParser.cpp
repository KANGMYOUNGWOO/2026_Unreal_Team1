
#include "PinballLikeEditor/Public/Relic/PBRelicTableParser.h"
#include "PBSheetParserUtils.h"
#include "PinBallLike/Struct/Relic//PBRelicTableRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBRelicTableParser::GetParserName() const
{
	return TEXT("Relic");
}

UScriptStruct* UPBRelicTableParser::GetRowStruct() const
{
	return FPBRelicTableRow::StaticStruct();
}

bool UPBRelicTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBRelicTableRow NewRow;

	NewRow.DisplayName =
		FText::FromString(RowData.FindRef(TEXT("DisplayName")));

	NewRow.Description =
		FText::FromString(RowData.FindRef(TEXT("Description")));

	NewRow.Rarity =
		ParseEnumValue(
			RowData.FindRef(TEXT("Rarity")),
			EPBRelicRarity::Common);

	NewRow.EffectType =
		ParseEnumValue(
			RowData.FindRef(TEXT("EffectType")),
			EPBRelicEffectType::None);

	NewRow.EffectValue =
		ParseFloatValue(
			RowData.FindRef(TEXT("EffectValue")),
			0.f);

	TargetTable->AddRow(RowName, NewRow);

	return true;
}