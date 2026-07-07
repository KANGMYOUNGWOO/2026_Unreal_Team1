// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"
#include "PBTableDataSubsystem.generated.h"

class UDataTable;

UCLASS()
class PINBALLLIKE_API UPBTableDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool IsTableDataReady() const;

	// 로딩 Subsystem이 준비한 테이블을 주입한다. 이 Subsystem은 조회 책임만 가진다.
	void SetCollectionTable(UDataTable* InCollectionTable);
	bool IsCollectionTableReady() const;
	bool FindCollectionRow(FName RowName, FPBCollectionTableRow& OutRow) const;
	void GetAllCollectionRows(TArray<FPBCollectionTableRow>& OutRows) const;

	void SetBumperTables(UDataTable* InBumperTable, UDataTable* InBumperTriggerTable, UDataTable* InBumperEffectTable);

#pragma region Bumper
	bool FindBumperRow(FName RowName, FPBBumperTableRow& OutRow) const;
	bool FindBumperTriggerRow(FName RowName, FPBBumperTriggerRow& OutRow) const;
	bool FindBumperEffectRow(FName RowName, FPBBumperEffectRow& OutRow) const;
	bool FindLinkedBumperTriggerRow(FName BumperRowName, FPBBumperTriggerRow& OutRow) const;
	bool FindLinkedBumperEffectRow(FName BumperRowName, FPBBumperEffectRow& OutRow) const;
#pragma endregion

private:
#pragma region Collection
	UPROPERTY()
	TObjectPtr<UDataTable> CollectionTable;
#pragma endregion

	template <typename RowType>
	bool FindTableRow(const UDataTable* Table, FName RowName, RowType& OutRow, const TCHAR* Context) const;

#pragma region Bumper
	UPROPERTY()
	TObjectPtr<UDataTable> BumperTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BumperTriggerTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BumperEffectTable;
#pragma endregion
};

template <typename RowType>
bool UPBTableDataSubsystem::FindTableRow(
	const UDataTable* Table,
	const FName RowName,
	RowType& OutRow,
	const TCHAR* Context) const
{
	if (!IsValid(Table) || RowName.IsNone())
	{
		return false;
	}

	const RowType* Row = Table->FindRow<RowType>(RowName, Context);
	if (!Row)
	{
		return false;
	}

	OutRow = *Row;
	return true;
}
