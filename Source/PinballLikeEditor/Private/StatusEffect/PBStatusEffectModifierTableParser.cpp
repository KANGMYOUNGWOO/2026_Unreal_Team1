// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusEffect/PBStatusEffectModifierTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectModifierRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBStatusEffectModifierTableParser::GetParserName() const
{
	return TEXT("StatusEffectModifier");
}

UScriptStruct* UPBStatusEffectModifierTableParser::GetRowStruct() const
{
	return FPBStatusEffectModifierRow::StaticStruct();
}

bool UPBStatusEffectModifierTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBStatusEffectModifierRow NewRow;
	NewRow.StatusEffectId = FName(*TrimCell(RowData.FindRef(TEXT("StatusEffectId"))));
	NewRow.ModifyType = ParseEnumValue(RowData.FindRef(TEXT("ModifyType")), EPBStatusEffectModifyType::Add);
	NewRow.Value = ParseFloatValue(RowData.FindRef(TEXT("Value")), 0.0f);
	NewRow.bScaleWithStack = ParseBoolValue(RowData.FindRef(TEXT("bScaleWithStack")), false);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
