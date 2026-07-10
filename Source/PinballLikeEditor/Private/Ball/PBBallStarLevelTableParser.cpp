// Fill out your copyright notice in the Description page of Project Settings.


#include "Ball/PBBallStarLevelTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Table/Ball/Struct/PBBallStarLevelRow.h"

using namespace PBSheetParserUtils;

void UPBBallStarLevelTableParser::AddParsedValue(TMap<FName, int32>& Values, const FName ValueName, const TMap<FString, FString>& RowData)
{
	if (ValueName.IsNone())
	{
		return;
	}
	Values.Add(ValueName, ParseIntValue(RowData.FindRef(ValueName.ToString()), 0));
}

UPBBallStarLevelTableParser::UPBBallStarLevelTableParser()
{
}

const TCHAR* UPBBallStarLevelTableParser::GetParserName() const
{
	return TEXT("BallStarLevel");
}

UScriptStruct* UPBBallStarLevelTableParser::GetRowStruct() const
{
	return FPBBallStarLevelRow::StaticStruct();
}

bool UPBBallStarLevelTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBallStarLevelRow NewRow;
	NewRow.BallId = FName(*TrimCell(RowData.FindRef(TEXT("BallId"))));
	NewRow.StarLevel = ParseIntValue(RowData.FindRef(TEXT("StarLevel")), 0);

	NewRow.BaseResources.Add(PBResourceNames::Health, ParseIntValue(RowData.FindRef(TEXT("Health")), 0));
	NewRow.BaseResources.Add(PBResourceNames::Mana, ParseIntValue(RowData.FindRef(TEXT("Mana")), 0));

	AddParsedValue(NewRow.BaseStats, PBStatNames::ManaRegen, RowData);
	AddParsedValue(NewRow.BaseStats, PBStatNames::Attack, RowData);
	AddParsedValue(NewRow.BaseStats, PBStatNames::Mass, RowData);
	AddParsedValue(NewRow.BaseStats, PBStatNames::Bounciness, RowData);
	AddParsedValue(NewRow.BaseStats, PBStatNames::Size, RowData);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
