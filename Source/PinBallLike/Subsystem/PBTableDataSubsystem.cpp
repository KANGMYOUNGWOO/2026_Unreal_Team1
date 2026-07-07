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
		&& IsValid(BumperEffectTable);
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

#pragma region Bumper

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
