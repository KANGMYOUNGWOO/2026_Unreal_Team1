// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "PinBallLike/Table/Ball/Struct/PBBallStarLevelRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossHitPointTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossPatternTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossTableRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectRow.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectSetRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"
#include "PinBallLike/Table/Shop/Struct/PBShopTableRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectModifierRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectTriggerRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicModifierRow.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTriggerTableRow.h"
#include "PBTableDataSubsystem.generated.h"

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
	bool HasStartupGameDataLoadCompleted() const { return bStartupGameDataLoadCompleted; }
	bool IsTableDataReady() const;
	bool IsBumperTableReady() const;
	bool IsCollectionCatalogDataReady() const;

	// 로딩 Subsystem이 준비한 테이블을 주입한다. 이 Subsystem은 조회 책임만 가진다.
	void SetCollectionTable(UDataTable* InCollectionTable);
	bool IsCollectionTableReady() const;
	bool FindCollectionRow(FName RowName, FPBCollectionTableRow& OutRow) const;
	void GetAllCollectionRows(TArray<FPBCollectionTableRow>& OutRows) const;

	void SetBumperTables(UDataTable* InBumperTable, UDataTable* InBumperTriggerTable, UDataTable* InBumperEffectTable);
	void SetGameplayEffectTables(UDataTable* InGameplayEffectTable, UDataTable* InGameplayEffectParamTable);
	void SetBallTables(UDataTable* InBallTable, UDataTable* InBallStarLevelTable);
	void SetSkillTable(UDataTable* InSkillTable);
	void SetBossTables(UDataTable* InBossTable, UDataTable* InBossHitPointTable, UDataTable* InBossPatternTable);
	void SetRelicTable(UDataTable* InRelicTable , UDataTable* InRelicModifierTable);
	void SetShopTable(UDataTable* InShopTable);
	void SetStatusEffectTables(
		UDataTable* InStatusEffectTable,
		UDataTable* InStatusEffectModifierTable,
		UDataTable* InStatusEffectTriggerTable);
	void SetEffectTables(
		UDataTable* InEffectTable,
		UDataTable* InEffectSetTable,
		UDataTable* InEffectParamTable);
	void SetSynergyTables(
		UDataTable* InSynergyTable,
		UDataTable* InSynergyTierTable);

	UPROPERTY(BlueprintAssignable, Category = "TableData")
	FPBTableDataLoadEvent OnStartupGameDataLoaded;

private:
	void OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths);

	UPROPERTY()
	TObjectPtr<UDataTable> CollectionTable;

	TSharedPtr<FStreamableHandle> StartupGameDataLoadHandle;
	bool bStartupGameDataLoadCompleted = false;

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
	bool FindGameplayEffectRow(FName RowName, FPBGameplayEffectRow& OutRow) const;
	bool GetGameplayEffectParamRows(FName EffectId, TArray<FPBGameplayEffectParamRow>& OutRows) const;
private:

	UPROPERTY()
	TObjectPtr<UDataTable> BumperTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BumperTriggerTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BumperEffectTable;

	UPROPERTY()
	TObjectPtr<UDataTable> GameplayEffectTable;

	UPROPERTY()
	TObjectPtr<UDataTable> GameplayEffectParamTable;

#pragma endregion

#pragma region Ball

public:
	bool GetAllBallRows(TArray<FName>& OutRowNames, TArray<FPBBallTableRow>& OutRows) const;
	bool FindBallRow(FName RowName, FPBBallTableRow& OutRow) const;
	bool FindBallStarLevelRow(FName RowName, FPBBallStarLevelRow& OutRow) const;
	bool FindBallStarLevelRow(FName BallId, int32 StarLevel, FName& OutRowName, FPBBallStarLevelRow& OutRow) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> BallTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BallStarLevelTable;

#pragma endregion

#pragma region Skill

public:
	bool GetAllSkillRows(TArray<FName>& OutRowNames, TArray<FPBBallSkillTableRow>& OutRows) const;
	bool FindSkillRow(FName RowName, FPBBallSkillTableRow& OutRow) const;
	bool FindDefaultSkillRowForBall(FName BallId, FPBBallSkillTableRow& OutRow) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> SkillTable;

#pragma endregion

#pragma region Shop

public:
	
	
	void GetAllShopRowName(TArray<FName>& OutShopIds);
	bool FindShopRow(FName RowName, FPBShopTableRow& OutRow) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> ShopTable;

#pragma endregion

#pragma  region Relic 
public:
	bool FindRelicRow(FName RowName,FPBRelicTableRow& OutRow) const;
	bool FindRelicModifierRow(FName RowName, FPBRelicModifierRow OutRow) const;
	bool FindRelicTrigger(FName RowName,FPBRelicTriggerTableRow& OutRow) const;
	
	void GetRelicModifierRows(FName RelicId, TArray<FPBRelicModifierRow>& OutRows) const;
	void GetAllRelicIds(TArray<FName>& OutRelicIds) const;
	
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> RelicTable;
	
	UPROPERTY()
	TObjectPtr<UDataTable> RelicModifierTable;
	
	UPROPERTY()
	TObjectPtr<UDataTable> RelicTriggerTable;
	
#pragma  endregion 
	
#pragma region Boss

public:
	bool GetBossRowNames(TArray<FName>& OutRowNames) const;
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

#pragma region Effect

public:
	bool FindEffectRow(FName RowName, FPBEffectTableRow& OutRow) const;
	bool GetEffectSetRows(FName EffectSetId, TArray<FPBEffectSetRow>& OutRows) const;
	bool GetEffectParamRows(FName EffectId, TArray<FPBEffectParamRow>& OutRows) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> EffectTable;

	UPROPERTY()
	TObjectPtr<UDataTable> EffectSetTable;

	UPROPERTY()
	TObjectPtr<UDataTable> EffectParamTable;

#pragma endregion

#pragma region Synergy

public:
	bool GetAllSynergyRows(TArray<FName>& OutRowNames, TArray<FPBSynergyTableRow>& OutRows) const;
	bool FindSynergyRow(FName RowName, FPBSynergyTableRow& OutRow) const;
	bool FindSynergyTierRow(FName RowName, FPBSynergyTierRow& OutRow) const;
	bool GetSynergyTierRows(FName SynergyId, TArray<FPBSynergyTierRow>& OutRows) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> SynergyTable;

	UPROPERTY()
	TObjectPtr<UDataTable> SynergyTierTable;

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
