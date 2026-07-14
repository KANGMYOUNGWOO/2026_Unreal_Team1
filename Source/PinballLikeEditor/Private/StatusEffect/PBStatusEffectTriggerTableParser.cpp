// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusEffect/PBStatusEffectTriggerTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectTriggerRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBStatusEffectTriggerTableParser::GetParserName() const
{
	return TEXT("StatusEffectTrigger");
}

UScriptStruct* UPBStatusEffectTriggerTableParser::GetRowStruct() const
{
	return FPBStatusEffectTriggerRow::StaticStruct();
}

bool UPBStatusEffectTriggerTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBStatusEffectTriggerRow NewRow;
	NewRow.StatusEffectId = FName(*TrimCell(RowData.FindRef(TEXT("StatusEffectId"))));
	NewRow.Value = ParseFloatValue(RowData.FindRef(TEXT("Value")), 0.0f);
	NewRow.bScaleWithStack = ParseBoolValue(RowData.FindRef(TEXT("bScaleWithStack")), false);
	NewRow.TargetStatusEffectId = FName(*TrimCell(RowData.FindRef(TEXT("TargetStatusEffectId"))));
	NewRow.TargetTag = FName(*TrimCell(RowData.FindRef(TEXT("TargetTag"))));
	NewRow.ProcChance = FMath::Clamp(ParseFloatValue(RowData.FindRef(TEXT("ProcChance")), 100.0f), 0.0f, 100.0f);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
