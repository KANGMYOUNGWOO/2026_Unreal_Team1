// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "PinBallLike/Table/Ball/Struct/PBBallStarLevelRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossHitPointTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossPatternTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossTableRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"
#include "PinBallLike/Table/Shop/Struct/PBShopTableRow.h"
#include "PBTableDataSubsystem.generated.h"

struct FPBStatusEffectTriggerRow;
struct FPBStatusEffectModifierRow;
struct FPBStatusEffectRow;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBTableDataLoadEvent);

UCLASS()
class PINBALLLIKE_API UPBTableDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// DeveloperSettings에 등록된 테이블을 비동기로 준비한다.
	void LoadStartupGameDataAsync();
	void UnloadStartupGameData();
	bool IsTableDataReady() const;

	// 로딩 Subsystem이 준비한 테이블을 주입한다. 이 Subsystem은 조회 책임만 가진다.
	void SetCollectionTable(UDataTable* InCollectionTable);
	bool IsCollectionTableReady() const;
	bool FindCollectionRow(FName RowName, FPBCollectionTableRow& OutRow) const;
	void GetAllCollectionRows(TArray<FPBCollectionTableRow>& OutRows) const;

	void SetBumperTables(UDataTable* InBumperTable, UDataTable* InBumperTriggerTable, UDataTable* InBumperEffectTable);
	void SetBallTables(UDataTable* InBallTable, UDataTable* InBallStarLevelTable);
	void SetBossTables(UDataTable* InBossTable, UDataTable* InBossHitPointTable, UDataTable* InBossPatternTable);
	void SetStatusEffectTables(
		UDataTable* InStatusEffectTable,
		UDataTable* InStatusEffectModifierTable,
		UDataTable* InStatusEffectTriggerTable);

	UPROPERTY(BlueprintAssignable, Category = "TableData")
	FPBTableDataLoadEvent OnStartupGameDataLoaded;

private:
	void OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths);

	UPROPERTY()
	TObjectPtr<UDataTable> CollectionTable;

	TSharedPtr<FStreamableHandle> StartupGameDataLoadHandle;

	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedStartupTables;

	template <typename RowType>
	bool FindTableRow(const UDataTable* Table, FName RowName, RowType& OutRow, const TCHAR* Context) const;

	template <typename RowType>
	bool GetAllTableRows(
		const UDataTable* Table,
		TArray<FName>& OutRowNames,
		TArray<RowType>& OutRows,
		const TCHAR* Context) const;

#pragma region Bumper

public:
	bool GetAllBumperRows(TArray<FName>& OutRowNames, TArray<FPBBumperTableRow>& OutRows) const;

	bool FindBumperRow(FName RowName, FPBBumperTableRow& OutRow) const;
	bool FindBumperTriggerRow(FName RowName, FPBBumperTriggerRow& OutRow) const;
	bool FindBumperEffectRow(FName RowName, FPBBumperEffectRow& OutRow) const;
	bool FindLinkedBumperTriggerRow(FName BumperRowName, FPBBumperTriggerRow& OutRow) const;
	bool FindLinkedBumperEffectRow(FName BumperRowName, FPBBumperEffectRow& OutRow) const;
private:

	UPROPERTY()
	TObjectPtr<UDataTable> BumperTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BumperTriggerTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BumperEffectTable;

#pragma endregion

#pragma region Ball

public:
	bool FindBallRow(FName RowName, FPBBallTableRow& OutRow) const;
	bool FindBallStarLevelRow(FName RowName, FPBBallStarLevelRow& OutRow) const;
	bool FindBallStarLevelRow(FName BallId, int32 StarLevel, FName& OutRowName, FPBBallStarLevelRow& OutRow) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> BallTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BallStarLevelTable;

#pragma endregion

#pragma region Shop

public:
	bool FindShopRow(FName RowName, FPBShopTableRow& OutRow) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> ShopTable;

#pragma endregion

#pragma region Boss

public:
	bool FindBossRow(FName RowName, FPBBossTableRow& OutRow) const;
	bool FindBossHitPointRow(FName RowName, FPBBossHitPointTableRow& OutRow) const;
	bool FindBossPatternRow(FName RowName, FPBBossPatternTableRow& OutRow) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> BossTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BossHitPointTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BossPatternTable;

#pragma endregion

#pragma region StatusEffect

public:
	bool FindStatusEffectRow(FName RowName, FPBStatusEffectRow& OutRow) const;
	bool GetStatusEffectModifierRows(FName StatusEffectId, TArray<FPBStatusEffectModifierRow>& OutRows) const;
	bool GetStatusEffectTriggerRows(FName StatusEffectId, TArray<FPBStatusEffectTriggerRow>& OutRows) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> StatusEffectTable;

	UPROPERTY()
	TObjectPtr<UDataTable> StatusEffectModifierTable;

	UPROPERTY()
	TObjectPtr<UDataTable> StatusEffectTriggerTable;

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

template <typename RowType>
bool UPBTableDataSubsystem::GetAllTableRows(
	const UDataTable* Table,
	TArray<FName>& OutRowNames,
	TArray<RowType>& OutRows,
	const TCHAR* Context) const
{
	OutRowNames.Reset();
	OutRows.Reset();

	if (!IsValid(Table))
	{
		return false;
	}

	Table->ForeachRow<RowType>(
		Context,
		[&OutRowNames, &OutRows](const FName& RowName, const RowType& Row)
		{
			OutRowNames.Add(RowName);
			OutRows.Add(Row);
		});

	return OutRows.Num() > 0;
}
