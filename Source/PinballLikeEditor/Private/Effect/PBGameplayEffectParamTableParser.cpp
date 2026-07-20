#include "Effect/PBGameplayEffectParamTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectParamRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBGameplayEffectParamTableParser::GetParserName() const
{
	return TEXT("GameplayEffectParam");
}

UScriptStruct* UPBGameplayEffectParamTableParser::GetRowStruct() const
{
	return FPBGameplayEffectParamRow::StaticStruct();
}

bool UPBGameplayEffectParamTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBGameplayEffectParamRow NewRow;
	NewRow.EffectId = ParseNameValue(RowData.FindRef(TEXT("EffectId")));
	NewRow.ParamKey = ParseNameValue(RowData.FindRef(TEXT("ParamKey")));
	NewRow.ParamValue = TrimCell(RowData.FindRef(TEXT("ParamValue")));

	if (NewRow.EffectId.IsNone() || NewRow.ParamKey.IsNone())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Sheet][GameplayEffectParam] Skip invalid row. RowName=%s EffectId=%s ParamKey=%s"),
			*RowName.ToString(),
			*NewRow.EffectId.ToString(),
			*NewRow.ParamKey.ToString());
		return false;
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
