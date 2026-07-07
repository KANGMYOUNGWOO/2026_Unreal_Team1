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
