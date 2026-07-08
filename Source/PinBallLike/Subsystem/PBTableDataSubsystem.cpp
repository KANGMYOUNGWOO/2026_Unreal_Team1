// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTableDataSubsystem.h"

#include "Engine/DataTable.h"

void UPBTableDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

bool UPBTableDataSubsystem::IsTableDataReady() const
{
	return IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable)
		&& IsValid(BallTable)
		&& IsValid(BallStarLevelTable);
}

void UPBTableDataSubsystem::SetCollectionTable(UDataTable* InCollectionTable)
{
	CollectionTable = InCollectionTable;

	UE_LOG(LogTemp, Log, TEXT("[TableData] Collection table assigned. Collection=%s"),
		*GetNameSafe(CollectionTable));
}

bool UPBTableDataSubsystem::IsCollectionTableReady() const
{
	return IsValid(CollectionTable);
}

bool UPBTableDataSubsystem::FindCollectionRow(const FName RowName, FPBCollectionTableRow& OutRow) const
{
	return FindTableRow(CollectionTable, RowName, OutRow, TEXT("FindCollectionRow"));
}

void UPBTableDataSubsystem::GetAllCollectionRows(TArray<FPBCollectionTableRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(CollectionTable))
	{
		return;
	}

	TArray<FPBCollectionTableRow*> Rows;
	CollectionTable->GetAllRows<FPBCollectionTableRow>(TEXT("GetAllCollectionRows"), Rows);
	for (const FPBCollectionTableRow* Row : Rows)
	{
		if (Row && !Row->CollectionId.IsNone())
		{
			OutRows.Add(*Row);
		}
	}
}

void UPBTableDataSubsystem::SetBumperTables(
	UDataTable* InBumperTable,
	UDataTable* InBumperTriggerTable,
	UDataTable* InBumperEffectTable)
{
	BumperTable = InBumperTable;
	BumperTriggerTable = InBumperTriggerTable;
	BumperEffectTable = InBumperEffectTable;

	UE_LOG(LogTemp, Log, TEXT("[TableData] Bumper tables assigned. Bumper=%s Trigger=%s Effect=%s"),
		*GetNameSafe(BumperTable),
		*GetNameSafe(BumperTriggerTable),
		*GetNameSafe(BumperEffectTable));
}

void UPBTableDataSubsystem::SetBallTables(UDataTable* InBallTable, UDataTable* InBallStarLevelTable)
{
	BallTable = InBallTable;
	BallStarLevelTable = InBallStarLevelTable;
	
	UE_LOG(LogTemp, Log, TEXT("[TableData] Ball tables assigned. Ball=%s StarLevel=%s"),
	*GetNameSafe(BallTable),
	*GetNameSafe(BallStarLevelTable));
}

#pragma region Bumper

bool UPBTableDataSubsystem::GetAllBumperRows(
	TArray<FName>& OutRowNames,
	TArray<FPBBumperTableRow>& OutRows) const
{
	return GetAllTableRows(BumperTable, OutRowNames, OutRows, TEXT("GetAllBumperRows"));
}

bool UPBTableDataSubsystem::FindBumperRow(const FName RowName, FPBBumperTableRow& OutRow) const
{
	return FindTableRow(BumperTable, RowName, OutRow, TEXT("FindBumperRow"));
}

bool UPBTableDataSubsystem::FindBumperTriggerRow(const FName RowName, FPBBumperTriggerRow& OutRow) const
{
	return FindTableRow(BumperTriggerTable, RowName, OutRow, TEXT("FindBumperTriggerRow"));
}

bool UPBTableDataSubsystem::FindBumperEffectRow(const FName RowName, FPBBumperEffectRow& OutRow) const
{
	return FindTableRow(BumperEffectTable, RowName, OutRow, TEXT("FindBumperEffectRow"));
}

bool UPBTableDataSubsystem::FindLinkedBumperTriggerRow(
	const FName BumperRowName,
	FPBBumperTriggerRow& OutRow) const
{
	FPBBumperTableRow BumperRow;
	if (!FindBumperRow(BumperRowName, BumperRow))
	{
		return false;
	}

	return FindBumperTriggerRow(BumperRow.TriggerID, OutRow);
}

bool UPBTableDataSubsystem::FindLinkedBumperEffectRow(
	const FName BumperRowName,
	FPBBumperEffectRow& OutRow) const
{
	FPBBumperTableRow BumperRow;
	if (!FindBumperRow(BumperRowName, BumperRow))
	{
		return false;
	}

	return FindBumperEffectRow(BumperRow.EffectID, OutRow);
}


#pragma endregion

#pragma region Ball

bool UPBTableDataSubsystem::FindBallRow(FName RowName, FPBBallTableRow& OutRow) const
{
	return FindTableRow(BallTable, RowName, OutRow, TEXT("FindBallRow"));
}

bool UPBTableDataSubsystem::FindBallStarLevelRow(FName RowName, FPBBallStarLevelRow& OutRow) const
{
	return FindTableRow(BallStarLevelTable, RowName, OutRow, TEXT("FindBallStarLevelRow"));
}

bool UPBTableDataSubsystem::FindBallStarLevelRow(
	const FName BallId,
	const int32 StarLevel,
	FName& OutRowName,
	FPBBallStarLevelRow& OutRow) const
{
	OutRowName = NAME_None;
	OutRow = FPBBallStarLevelRow();
	if (!IsValid(BallStarLevelTable) || BallId.IsNone() || StarLevel <= 0)
	{
		return false;
	}

	for (const TPair<FName, uint8*>& RowPair : BallStarLevelTable->GetRowMap())
	{
		const FPBBallStarLevelRow* Row = reinterpret_cast<FPBBallStarLevelRow*>(RowPair.Value);
		if (Row && Row->BallId == BallId && Row->StarLevel == StarLevel)
		{
			OutRowName = RowPair.Key;
			OutRow = *Row;
			return true;
		}
	}

	return false;
}

#pragma endregion 
