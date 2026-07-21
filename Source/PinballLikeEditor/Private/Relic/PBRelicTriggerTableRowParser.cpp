#include "PinballLikeEditor/Public/Relic/PBRelicTriggerTableRowParser.h"

#include "PBSheetParserUtils.h"

#include "PinBallLike/Table/Relic/Struct/PBRelicTriggerTableRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBRelicTriggerTableRowParser::GetParserName() const
{
	return TEXT("RelicTrigger");
}

UScriptStruct*  UPBRelicTriggerTableRowParser::GetRowStruct() const
{
	return FPBRelicTriggerTableRow::StaticStruct();
}

bool UPBRelicTriggerTableRowParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBRelicTriggerTableRow NewRow;

	NewRow.RelicId =
		FName(
			RowData.FindRef(TEXT("RelicId")));

	NewRow.TriggerTag =
		FGameplayTag::RequestGameplayTag(
			FName(
				RowData.FindRef(TEXT("TriggerTag"))));

	NewRow.ConditionType =
		ParseEnumValue(
			RowData.FindRef(TEXT("ConditionType")),
			ERelicTriggerCondition::None);

	NewRow.ConditionValue =
		FCString::Atoi(
			*RowData.FindRef(TEXT("ConditionValue")));

	NewRow.StatusEffectId =
		FName(
			RowData.FindRef(TEXT("StatusEffectId")));

	TargetTable->AddRow(
		RowName,
		NewRow);

	return true;
}