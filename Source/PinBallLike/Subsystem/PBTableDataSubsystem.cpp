// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTableDataSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectModifierRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectTriggerRow.h"

void UPBTableDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadStartupGameDataAsync();
}

void UPBTableDataSubsystem::Deinitialize()
{
	UnloadStartupGameData();

	Super::Deinitialize();
}

void UPBTableDataSubsystem::LoadStartupGameDataAsync()
{
	UnloadStartupGameData();

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (!IsValid(Settings))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TableData] Missing PBGameDataSettings."));
		return;
	}

	TArray<FSoftObjectPath> TablePaths;
	TablePaths.Reserve(13);

	// 테이블 경로는 DeveloperSettings에서 관리한다.
	const FSoftObjectPath CollectionTablePath = Settings->CollectionTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTablePath = Settings->BumperTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTriggerTablePath = Settings->BumperTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath BumperEffectTablePath = Settings->BumperEffectTable.ToSoftObjectPath();
	const FSoftObjectPath BallTablePath = Settings->BallTable.ToSoftObjectPath();
	const FSoftObjectPath BallStarLevelTablePath = Settings->BallStarLevelTable.ToSoftObjectPath();
	const FSoftObjectPath SkillTablePath = Settings->SkillTable.ToSoftObjectPath();
	const FSoftObjectPath BossTablePath = Settings->Boss.ToSoftObjectPath();
	const FSoftObjectPath BossHitPointTablePath = Settings->BossHitPoint.ToSoftObjectPath();
	const FSoftObjectPath BossPatternTablePath = Settings->BossPattern.ToSoftObjectPath();
	const FSoftObjectPath StatusEffectTablePath = Settings->StatusEffectTable.ToSoftObjectPath();
	const FSoftObjectPath StatusEffectModifierTablePath = Settings->StatusEffectModifierTable.ToSoftObjectPath();
	const FSoftObjectPath StatusEffectTriggerTablePath = Settings->StatusEffectTriggerTable.ToSoftObjectPath();

	if (CollectionTablePath.IsValid())
	{
		TablePaths.Add(CollectionTablePath);
	}

	if (BumperTablePath.IsValid())
	{
		TablePaths.Add(BumperTablePath);
	}

	if (BumperTriggerTablePath.IsValid())
	{
		TablePaths.Add(BumperTriggerTablePath);
	}

	if (BumperEffectTablePath.IsValid())
	{
		TablePaths.Add(BumperEffectTablePath);
	}

	if (BallTablePath.IsValid())
	{
		TablePaths.Add(BallTablePath);
	}

	if (BallStarLevelTablePath.IsValid())
	{
		TablePaths.Add(BallStarLevelTablePath);
	}

	if (SkillTablePath.IsValid())
	{
		TablePaths.Add(SkillTablePath);
	}

	if (BossTablePath.IsValid())
	{
		TablePaths.Add(BossTablePath);
	}

	if (BossHitPointTablePath.IsValid())
	{
		TablePaths.Add(BossHitPointTablePath);
	}

	if (BossPatternTablePath.IsValid())
	{
		TablePaths.Add(BossPatternTablePath);
	}

	if (StatusEffectTablePath.IsValid())
	{
		TablePaths.Add(StatusEffectTablePath);
	}

	if (StatusEffectModifierTablePath.IsValid())
	{
		TablePaths.Add(StatusEffectModifierTablePath);
	}

	if (StatusEffectTriggerTablePath.IsValid())
	{
		TablePaths.Add(StatusEffectTriggerTablePath);
	}

	if (TablePaths.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TableData] No startup table paths are configured."));
		return;
	}

	StartupGameDataLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		TablePaths,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBTableDataSubsystem::OnStartupGameDataLoadedInternal,
			TablePaths));
}

void UPBTableDataSubsystem::UnloadStartupGameData()
{
	LoadedStartupTables.Empty();

	SetCollectionTable(nullptr);
	SetBumperTables(nullptr, nullptr, nullptr);
	SetBallTables(nullptr, nullptr);
	SetSkillTable(nullptr);
	SetBossTables(nullptr, nullptr, nullptr);
	SetStatusEffectTables(nullptr, nullptr, nullptr);

	if (StartupGameDataLoadHandle.IsValid())
	{
		StartupGameDataLoadHandle->ReleaseHandle();
		StartupGameDataLoadHandle.Reset();
	}
}

void UPBTableDataSubsystem::OnStartupGameDataLoadedInternal(TArray<FSoftObjectPath> LoadedPaths)
{
	LoadedStartupTables.Empty();

	UDataTable* LoadedCollectionTable = nullptr;
	UDataTable* LoadedBumperTable = nullptr;
	UDataTable* LoadedBumperTriggerTable = nullptr;
	UDataTable* LoadedBumperEffectTable = nullptr;
	UDataTable* LoadedBallTable = nullptr;
	UDataTable* LoadedBallStarLevelTable = nullptr;
	UDataTable* LoadedSkillTable = nullptr;
	UDataTable* LoadedBossTable = nullptr;
	UDataTable* LoadedBossHitPointTable = nullptr;
	UDataTable* LoadedBossPatternTable = nullptr;
	UDataTable* LoadedStatusEffectTable = nullptr;
	UDataTable* LoadedStatusEffectModifierTable = nullptr;
	UDataTable* LoadedStatusEffectTriggerTable = nullptr;

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (IsValid(Settings))
	{
		// RequestAsyncLoad 완료 후 실제 테이블 객체를 조회용 포인터에 연결한다.
		LoadedCollectionTable = Cast<UDataTable>(Settings->CollectionTable.Get());
		LoadedBumperTable = Cast<UDataTable>(Settings->BumperTable.Get());
		LoadedBumperTriggerTable = Cast<UDataTable>(Settings->BumperTriggerTable.Get());
		LoadedBumperEffectTable = Cast<UDataTable>(Settings->BumperEffectTable.Get());
		LoadedBallTable = Cast<UDataTable>(Settings->BallTable.Get());
		LoadedBallStarLevelTable = Cast<UDataTable>(Settings->BallStarLevelTable.Get());
		LoadedSkillTable = Cast<UDataTable>(Settings->SkillTable.Get());
		LoadedBossTable = Cast<UDataTable>(Settings->Boss.Get());
		LoadedBossHitPointTable = Cast<UDataTable>(Settings->BossHitPoint.Get());
		LoadedBossPatternTable = Cast<UDataTable>(Settings->BossPattern.Get());
		LoadedStatusEffectTable = Cast<UDataTable>(Settings->StatusEffectTable.Get());
		LoadedStatusEffectModifierTable = Cast<UDataTable>(Settings->StatusEffectModifierTable.Get());
		LoadedStatusEffectTriggerTable = Cast<UDataTable>(Settings->StatusEffectTriggerTable.Get());
	}

	for (const FSoftObjectPath& LoadedPath : LoadedPaths)
	{
		if (UDataTable* LoadedTable = Cast<UDataTable>(LoadedPath.ResolveObject()))
		{
			LoadedStartupTables.Add(LoadedTable);
		}
	}

	SetCollectionTable(LoadedCollectionTable);
	SetBumperTables(LoadedBumperTable, LoadedBumperTriggerTable, LoadedBumperEffectTable);
	SetBallTables(LoadedBallTable, LoadedBallStarLevelTable);
	SetSkillTable(LoadedSkillTable);
	SetBossTables(LoadedBossTable, LoadedBossHitPointTable, LoadedBossPatternTable);
	SetStatusEffectTables(LoadedStatusEffectTable, LoadedStatusEffectModifierTable, LoadedStatusEffectTriggerTable);

	UE_LOG(LogTemp, Log, TEXT("[TableData] Startup table data loaded. Ready=%s TableCount=%d"),
		IsTableDataReady() ? TEXT("true") : TEXT("false"),
		LoadedStartupTables.Num());

	OnStartupGameDataLoaded.Broadcast();
}

bool UPBTableDataSubsystem::IsTableDataReady() const
{
	return IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable)
		&& IsValid(BallTable)
		&& IsValid(BallStarLevelTable)
		&& IsValid(SkillTable)
		&& IsValid(BossTable)
		&& IsValid(BossHitPointTable)
		&& IsValid(BossPatternTable);
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

void UPBTableDataSubsystem::SetSkillTable(UDataTable* InSkillTable)
{
	SkillTable = InSkillTable;

	UE_LOG(LogTemp, Log, TEXT("[TableData] Skill table assigned. Skill=%s"),
		*GetNameSafe(SkillTable));
}

void UPBTableDataSubsystem::SetBossTables(
	UDataTable* InBossTable,
	UDataTable* InBossHitPointTable,
	UDataTable* InBossPatternTable)
{
	BossTable = InBossTable;
	BossHitPointTable = InBossHitPointTable;
	BossPatternTable = InBossPatternTable;

	UE_LOG(LogTemp, Log, TEXT("[TableData] Boss tables assigned. Boss=%s HitPoint=%s Pattern=%s"),
		*GetNameSafe(BossTable),
		*GetNameSafe(BossHitPointTable),
		*GetNameSafe(BossPatternTable));
}

void UPBTableDataSubsystem::SetStatusEffectTables(
	UDataTable* InStatusEffectTable,
	UDataTable* InStatusEffectModifierTable,
	UDataTable* InStatusEffectTriggerTable)
{
	StatusEffectTable = InStatusEffectTable;
	StatusEffectModifierTable = InStatusEffectModifierTable;
	StatusEffectTriggerTable = InStatusEffectTriggerTable;

	UE_LOG(LogTemp, Log, TEXT("[TableData] StatusEffect tables assigned. StatusEffect=%s Modifier=%s Trigger=%s"),
		*GetNameSafe(StatusEffectTable),
		*GetNameSafe(StatusEffectModifierTable),
		*GetNameSafe(StatusEffectTriggerTable));
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

#pragma region Skill

bool UPBTableDataSubsystem::GetAllSkillRows(
	TArray<FName>& OutRowNames,
	TArray<FPBBallSkillTableRow>& OutRows) const
{
	return GetAllTableRows(SkillTable, OutRowNames, OutRows, TEXT("GetAllSkillRows"));
}

bool UPBTableDataSubsystem::FindSkillRow(
	const FName RowName,
	FPBBallSkillTableRow& OutRow) const
{
	return FindTableRow(SkillTable, RowName, OutRow, TEXT("FindSkillRow"));
}

#pragma endregion

#pragma region Shop

bool UPBTableDataSubsystem::FindShopRow(FName RowName, FPBShopTableRow& OutRow) const
{
	return FindTableRow(ShopTable, RowName, OutRow, TEXT("FindShopRow"));
}

#pragma endregion

#pragma region Boss

bool UPBTableDataSubsystem::FindBossRow(FName RowName, FPBBossTableRow& OutRow) const
{
	return FindTableRow(BossTable, RowName, OutRow, TEXT("FindBossRow"));
}

bool UPBTableDataSubsystem::FindBossHitPointRow(FName RowName, FPBBossHitPointTableRow& OutRow) const
{
	return FindTableRow(BossHitPointTable, RowName, OutRow, TEXT("FindBossHitPointRow"));
}

bool UPBTableDataSubsystem::FindBossPatternRow(FName RowName, FPBBossPatternTableRow& OutRow) const
{
	return FindTableRow(BossPatternTable, RowName, OutRow, TEXT("FindBossPatternRow"));
}

#pragma endregion

#pragma region StatusEffect

bool UPBTableDataSubsystem::FindStatusEffectRow(const FName RowName, FPBStatusEffectRow& OutRow) const
{
	return FindTableRow(StatusEffectTable, RowName, OutRow, TEXT("FindStatusEffectRow"));
}

bool UPBTableDataSubsystem::GetStatusEffectModifierRows(
	const FName StatusEffectId,
	TArray<FPBStatusEffectModifierRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(StatusEffectModifierTable) || StatusEffectId.IsNone())
	{
		return false;
	}

	StatusEffectModifierTable->ForeachRow<FPBStatusEffectModifierRow>(
		TEXT("GetStatusEffectModifierRows"),
		[StatusEffectId, &OutRows](const FName& RowName, const FPBStatusEffectModifierRow& Row)
		{
			if (Row.StatusEffectId == StatusEffectId)
			{
				OutRows.Add(Row);
			}
		});

	return OutRows.Num() > 0;
}

bool UPBTableDataSubsystem::GetStatusEffectTriggerRows(
	const FName StatusEffectId,
	TArray<FPBStatusEffectTriggerRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(StatusEffectTriggerTable) || StatusEffectId.IsNone())
	{
		return false;
	}

	StatusEffectTriggerTable->ForeachRow<FPBStatusEffectTriggerRow>(
		TEXT("GetStatusEffectTriggerRows"),
		[StatusEffectId, &OutRows](const FName& RowName, const FPBStatusEffectTriggerRow& Row)
		{
			if (Row.StatusEffectId == StatusEffectId)
			{
				OutRows.Add(Row);
			}
		});

	return OutRows.Num() > 0;
}

#pragma endregion
