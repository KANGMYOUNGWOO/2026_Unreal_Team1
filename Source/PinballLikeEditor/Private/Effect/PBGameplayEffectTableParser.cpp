#include "Effect/PBGameplayEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBGameplayEffectTableParser::GetParserName() const
{
	return TEXT("GameplayEffect");
}

UScriptStruct* UPBGameplayEffectTableParser::GetRowStruct() const
{
	return FPBGameplayEffectRow::StaticStruct();
}

bool UPBGameplayEffectTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBGameplayEffectRow NewRow;
	NewRow.EffectType = ParseNameValue(RowData.FindRef(TEXT("EffectType")));
	NewRow.TargetType = ParseNameValue(RowData.FindRef(TEXT("TargetType")));
	NewRow.TargetFilter = ParseNameValue(RowData.FindRef(TEXT("TargetFilter")));
	NewRow.TriggerTag = ParseNameValue(RowData.FindRef(TEXT("TriggerTag")));
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));
	NewRow.LinkedStatusEffectId = ParseNameValue(RowData.FindRef(TEXT("LinkedStatusEffectId")));

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
