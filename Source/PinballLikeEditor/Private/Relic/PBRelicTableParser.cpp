
#include "PinballLikeEditor/Public/Relic/PBRelicTableParser.h"
#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PinBallLike/Struct/Relic/PBRelicTypes.h"

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
		FText::FromString(
			RowData.FindRef(TEXT("DisplayName")));

	NewRow.Description =
		FText::FromString(
			RowData.FindRef(TEXT("Description")));

	NewRow.Rarity =
		ParseEnumValue(
			RowData.FindRef(TEXT("Rarity")),
			EPBRelicRarity::Common);

	TargetTable->AddRow(RowName, NewRow);

	return true;
}
