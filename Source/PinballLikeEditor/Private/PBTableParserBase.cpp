// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTableParserBase.h"

#include "PBSheetParserUtils.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBTableParserBase::GetParserName() const
{
	return TEXT("Table");
}

UScriptStruct* UPBTableParserBase::GetRowStruct() const
{
	return nullptr;
}

bool UPBTableParserBase::ParseRow(FName RowName, const TMap<FString, FString>& RowData)
{
	(void)RowName;
	(void)RowData;
	return false;
}

void UPBTableParserBase::OnParseComplete()
{
	const TCHAR* ParserName = GetParserName();
	UE_LOG(LogTemp, Log, TEXT("[Sheet][%s] OnParseComplete. RowCount=%d"), ParserName, GetRowCount());
	LogHeaders(ParserName, GetHeaders());

	if (!ValidateTargetTable(TargetTable, GetRowStruct()))
	{
		LogTargetTableError(ParserName, TargetTable, GetRowStruct());
		return;
	}

	ResetTargetTable(TargetTable);
	int32 AddedRowCount = 0;

	for (int32 RowIndex = 0; RowIndex < GetRowCount(); ++RowIndex)
	{
		TMap<FString, FString> RowData;
		if (!GetRowAt(RowIndex, RowData))
		{
			continue;
		}

		const FString RowNameString = RowData.FindRef(TEXT("RowName"));
		if (IsUnsetValue(RowNameString))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Sheet][%s] Skip row %d because RowName is empty."),
				ParserName,
				RowIndex);
			continue;
		}

		const FName RowName(*TrimCell(RowNameString));
		if (ParseRow(RowName, RowData))
		{
			++AddedRowCount;
		}
	}

	MarkTargetTableDirty(TargetTable);
	UE_LOG(LogTemp, Log, TEXT("[BumperSheet][%s] Added %d rows to %s."),
		ParserName,
		AddedRowCount,
		*GetNameSafe(TargetTable));
}
