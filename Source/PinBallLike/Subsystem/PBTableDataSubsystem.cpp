// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTableDataSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"

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
		bStartupGameDataLoadCompleted = true;
		OnStartupGameDataLoaded.Broadcast();
		return;
	}

	TArray<FSoftObjectPath> TablePaths;
	TablePaths.Reserve(24);

	// 테이블 경로는 DeveloperSettings에서 관리한다.
	const FSoftObjectPath CollectionTablePath = Settings->CollectionTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTablePath = Settings->BumperTable.ToSoftObjectPath();
	const FSoftObjectPath BumperTriggerTablePath = Settings->BumperTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath BumperEffectTablePath = Settings->BumperEffectTable.ToSoftObjectPath();
	const FSoftObjectPath GameplayEffectTablePath = Settings->GameplayEffectTable.ToSoftObjectPath();
	const FSoftObjectPath GameplayEffectParamTablePath = Settings->GameplayEffectParamTable.ToSoftObjectPath();
	const FSoftObjectPath BallTablePath = Settings->BallTable.ToSoftObjectPath();
	const FSoftObjectPath BallStarLevelTablePath = Settings->BallStarLevelTable.ToSoftObjectPath();
	const FSoftObjectPath SkillTablePath = Settings->SkillTable.ToSoftObjectPath();
	const FSoftObjectPath BossTablePath = Settings->Boss.ToSoftObjectPath();
	const FSoftObjectPath BossHitPointTablePath = Settings->BossHitPoint.ToSoftObjectPath();
	const FSoftObjectPath BossPatternTablePath = Settings->BossPattern.ToSoftObjectPath();
	const FSoftObjectPath StatusEffectTablePath = Settings->StatusEffectTable.ToSoftObjectPath();
	const FSoftObjectPath StatusEffectModifierTablePath = Settings->StatusEffectModifierTable.ToSoftObjectPath();
	const FSoftObjectPath StatusEffectTriggerTablePath = Settings->StatusEffectTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath SynergyTablePath = Settings->SynergyTable.ToSoftObjectPath();
	const FSoftObjectPath SynergyTierTablePath = Settings->SynergyTierTable.ToSoftObjectPath();
	const FSoftObjectPath SynergyEffectTablePath = Settings->SynergyEffectTable.ToSoftObjectPath();
	const FSoftObjectPath SynergyEffectModifierTablePath = Settings->SynergyEffectModifierTable.ToSoftObjectPath();
	const FSoftObjectPath SynergyEffectTriggerTablePath = Settings->SynergyEffectTriggerTable.ToSoftObjectPath();
	const FSoftObjectPath ShopTablePath = Settings->ShopTable.ToSoftObjectPath();
	const FSoftObjectPath RelicTablePath = Settings->RelicTable.ToSoftObjectPath();
    const FSoftObjectPath RelicModifierTablePath = Settings->RelicModifierTable.ToSoftObjectPath();
	
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

	if (GameplayEffectTablePath.IsValid())
	{
		TablePaths.Add(GameplayEffectTablePath);
	}

	if (GameplayEffectParamTablePath.IsValid())
	{
		TablePaths.Add(GameplayEffectParamTablePath);
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

	if (SynergyTablePath.IsValid())
	{
		TablePaths.Add(SynergyTablePath);
	}

	if (SynergyTierTablePath.IsValid())
	{
		TablePaths.Add(SynergyTierTablePath);
	}

	if (SynergyEffectTablePath.IsValid())
	{
		TablePaths.Add(SynergyEffectTablePath);
	}

	if (SynergyEffectModifierTablePath.IsValid())
	{
		TablePaths.Add(SynergyEffectModifierTablePath);
	}

	if (SynergyEffectTriggerTablePath.IsValid())
	{
		TablePaths.Add(SynergyEffectTriggerTablePath);
	}
	
	if(ShopTablePath.IsValid())
	{
		TablePaths.Add(ShopTablePath);
	}

	if(RelicTablePath.IsValid())
	{
		TablePaths.Add(RelicTablePath);
	}
	
	if (RelicModifierTablePath.IsValid())
	{
		TablePaths.Add(RelicModifierTablePath);
	}
	
	if (TablePaths.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TableData] No startup table paths are configured."));
		bStartupGameDataLoadCompleted = true;
		OnStartupGameDataLoaded.Broadcast();
		return;
	}

	StartupGameDataLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		TablePaths,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBTableDataSubsystem::OnStartupGameDataLoadedInternal,
			TablePaths));
	if (!StartupGameDataLoadHandle.IsValid() && !bStartupGameDataLoadCompleted)
	{
		UE_LOG(LogTemp, Error, TEXT("[TableData] Failed to create the startup table load request."));
		bStartupGameDataLoadCompleted = true;
		OnStartupGameDataLoaded.Broadcast();
	}
}

void UPBTableDataSubsystem::UnloadStartupGameData()
{
	bStartupGameDataLoadCompleted = false;
	LoadedStartupTables.Empty();

	SetCollectionTable(nullptr);
	SetBumperTables(nullptr, nullptr, nullptr);
	SetGameplayEffectTables(nullptr, nullptr);
	SetBallTables(nullptr, nullptr);
	SetSkillTable(nullptr);
	SetBossTables(nullptr, nullptr, nullptr);
	SetStatusEffectTables(nullptr, nullptr, nullptr);
	SetRelicTable(nullptr,nullptr);
	SetShopTable(nullptr);
	
	SetSynergyTables(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

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
	UDataTable* LoadedGameplayEffectTable = nullptr;
	UDataTable* LoadedGameplayEffectParamTable = nullptr;
	UDataTable* LoadedBallTable = nullptr;
	UDataTable* LoadedBallStarLevelTable = nullptr;
	UDataTable* LoadedSkillTable = nullptr;
	UDataTable* LoadedBossTable = nullptr;
	UDataTable* LoadedBossHitPointTable = nullptr;
	UDataTable* LoadedBossPatternTable = nullptr;
	UDataTable* LoadedStatusEffectTable = nullptr;
	UDataTable* LoadedStatusEffectModifierTable = nullptr;
	UDataTable* LoadedStatusEffectTriggerTable = nullptr;
	UDataTable* LoadedRelicTable = nullptr;
	UDataTable* LoadedRelicModifierTable = nullptr;
	
	UDataTable* LoadedSynergyTable = nullptr;
	UDataTable* LoadedSynergyTierTable = nullptr;
	UDataTable* LoadedSynergyTierEffectTable = nullptr;
	UDataTable* LoadedSynergyEffectTable = nullptr;
	UDataTable* LoadedSynergyEffectModifierTable = nullptr;
	UDataTable* LoadedSynergyEffectTriggerTable = nullptr;
	UDataTable* LoadedShopTable = nullptr;
	
	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (IsValid(Settings))
	{
		// RequestAsyncLoad 완료 후 실제 테이블 객체를 조회용 포인터에 연결한다.
		LoadedCollectionTable = Cast<UDataTable>(Settings->CollectionTable.Get());
		LoadedBumperTable = Cast<UDataTable>(Settings->BumperTable.Get());
		LoadedBumperTriggerTable = Cast<UDataTable>(Settings->BumperTriggerTable.Get());
		LoadedBumperEffectTable = Cast<UDataTable>(Settings->BumperEffectTable.Get());
		LoadedGameplayEffectTable = Cast<UDataTable>(Settings->GameplayEffectTable.Get());
		LoadedGameplayEffectParamTable = Cast<UDataTable>(Settings->GameplayEffectParamTable.Get());
		LoadedBallTable = Cast<UDataTable>(Settings->BallTable.Get());
		LoadedBallStarLevelTable = Cast<UDataTable>(Settings->BallStarLevelTable.Get());
		LoadedSkillTable = Cast<UDataTable>(Settings->SkillTable.Get());
		LoadedBossTable = Cast<UDataTable>(Settings->Boss.Get());
		LoadedBossHitPointTable = Cast<UDataTable>(Settings->BossHitPoint.Get());
		LoadedBossPatternTable = Cast<UDataTable>(Settings->BossPattern.Get());
		LoadedStatusEffectTable = Cast<UDataTable>(Settings->StatusEffectTable.Get());
		LoadedStatusEffectModifierTable = Cast<UDataTable>(Settings->StatusEffectModifierTable.Get());
		LoadedStatusEffectTriggerTable = Cast<UDataTable>(Settings->StatusEffectTriggerTable.Get());
		LoadedRelicTable = Cast<UDataTable>(Settings->RelicTable.Get());
		LoadedRelicModifierTable = Cast<UDataTable>(Settings->RelicModifierTable.Get());
		LoadedSynergyTable = Cast<UDataTable>(Settings->SynergyTable.Get());
		LoadedSynergyTierTable = Cast<UDataTable>(Settings->SynergyTierTable.Get());
		LoadedSynergyEffectTable = Cast<UDataTable>(Settings->SynergyEffectTable.Get());
		LoadedSynergyEffectModifierTable = Cast<UDataTable>(Settings->SynergyEffectModifierTable.Get());
		LoadedSynergyEffectTriggerTable = Cast<UDataTable>(Settings->SynergyEffectTriggerTable.Get());
		LoadedShopTable = Cast<UDataTable>(Settings->ShopTable.Get());
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
	SetGameplayEffectTables(LoadedGameplayEffectTable, LoadedGameplayEffectParamTable);
	SetBallTables(LoadedBallTable, LoadedBallStarLevelTable);
	SetSkillTable(LoadedSkillTable);
	SetBossTables(LoadedBossTable, LoadedBossHitPointTable, LoadedBossPatternTable);
	SetStatusEffectTables(LoadedStatusEffectTable, LoadedStatusEffectModifierTable, LoadedStatusEffectTriggerTable);
	SetSynergyTables(
		LoadedSynergyTable,
		LoadedSynergyTierTable,
		LoadedSynergyTierEffectTable,
		LoadedSynergyEffectTable,
		LoadedSynergyEffectModifierTable,
		LoadedSynergyEffectTriggerTable);

	SetRelicTable(LoadedRelicTable,LoadedRelicModifierTable);
	SetShopTable(LoadedShopTable);
	
	UE_LOG(LogTemp, Log, TEXT("[TableData] Startup table data loaded. Ready=%s TableCount=%d"),
		IsTableDataReady() ? TEXT("true") : TEXT("false"),
		LoadedStartupTables.Num());

	bStartupGameDataLoadCompleted = true;
	OnStartupGameDataLoaded.Broadcast();
}

bool UPBTableDataSubsystem::IsTableDataReady() const
{
	return IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable)
		&& IsValid(GameplayEffectTable)
		&& IsValid(GameplayEffectParamTable)
		&& IsValid(BallTable)
		&& IsValid(BallStarLevelTable)
		&& IsValid(SkillTable)
		&& IsValid(BossTable)
		&& IsValid(BossHitPointTable)
		&& IsValid(BossPatternTable);
}

bool UPBTableDataSubsystem::IsBumperTableReady() const
{
	return IsValid(BumperTable);
}

bool UPBTableDataSubsystem::IsCollectionCatalogDataReady() const
{
	return IsValid(BumperTable)
		&& IsValid(BumperTriggerTable)
		&& IsValid(BumperEffectTable)
		&& IsValid(BallTable)
		&& IsValid(BallStarLevelTable)
		&& IsValid(SkillTable)
		&& IsValid(BossTable)
		&& IsValid(BossHitPointTable)
		&& IsValid(BossPatternTable)
		&& IsValid(RelicTable)
		&& IsValid(RelicModifierTable)
		&& IsValid(SynergyTable)
		&& IsValid(SynergyTierTable)
		&& IsValid(SynergyEffectTable)
		&& IsValid(SynergyEffectModifierTable)
		&& IsValid(SynergyEffectTriggerTable);
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

void UPBTableDataSubsystem::SetGameplayEffectTables(
	UDataTable* InGameplayEffectTable,
	UDataTable* InGameplayEffectParamTable)
{
	GameplayEffectTable = InGameplayEffectTable;
	GameplayEffectParamTable = InGameplayEffectParamTable;

	UE_LOG(LogTemp, Log,
		TEXT("[TableData] Gameplay effect tables assigned. Effect=%s Param=%s"),
		*GetNameSafe(GameplayEffectTable),
		*GetNameSafe(GameplayEffectParamTable));
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

void UPBTableDataSubsystem::SetRelicTable(UDataTable* InRelicTable , UDataTable*  InRelicModifierTable)
{
	RelicTable = InRelicTable;
	RelicModifierTable = InRelicModifierTable;
	UE_LOG(LogTemp, Log, TEXT("[TableData] Relic tables assigned."));
}

void UPBTableDataSubsystem::SetShopTable(UDataTable* InShopTable)
{
	ShopTable = InShopTable;
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

void UPBTableDataSubsystem::SetSynergyTables(
	UDataTable* InSynergyTable,
	UDataTable* InSynergyTierTable,
	UDataTable* InSynergyTierEffectTable,
	UDataTable* InSynergyEffectTable,
	UDataTable* InSynergyEffectModifierTable,
	UDataTable* InSynergyEffectTriggerTable)
{
	SynergyTable = InSynergyTable;
	SynergyTierTable = InSynergyTierTable;
	SynergyTierEffectTable = InSynergyTierEffectTable;
	SynergyEffectTable = InSynergyEffectTable;
	SynergyEffectModifierTable = InSynergyEffectModifierTable;
	SynergyEffectTriggerTable = InSynergyEffectTriggerTable;

	UE_LOG(LogTemp, Log, TEXT("[TableData] Synergy tables assigned. Synergy=%s Tier=%s TierEffect=%s Effect=%s Modifier=%s Trigger=%s"),
		*GetNameSafe(SynergyTable),
		*GetNameSafe(SynergyTierTable),
		*GetNameSafe(SynergyTierEffectTable),
		*GetNameSafe(SynergyEffectTable),
		*GetNameSafe(SynergyEffectModifierTable),
		*GetNameSafe(SynergyEffectTriggerTable));
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

bool UPBTableDataSubsystem::FindGameplayEffectRow(
	const FName RowName,
	FPBGameplayEffectRow& OutRow) const
{
	return FindTableRow(
		GameplayEffectTable,
		RowName,
		OutRow,
		TEXT("FindGameplayEffectRow"));
}

bool UPBTableDataSubsystem::GetGameplayEffectParamRows(
	const FName EffectId,
	TArray<FPBGameplayEffectParamRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(GameplayEffectParamTable) || EffectId.IsNone())
	{
		return false;
	}

	TArray<FPBGameplayEffectParamRow*> Rows;
	GameplayEffectParamTable->GetAllRows<FPBGameplayEffectParamRow>(
		TEXT("GetGameplayEffectParamRows"),
		Rows);
	for (const FPBGameplayEffectParamRow* Row : Rows)
	{
		if (Row && Row->EffectId == EffectId && !Row->ParamKey.IsNone())
		{
			OutRows.Add(*Row);
		}
	}

	return !OutRows.IsEmpty();
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

bool UPBTableDataSubsystem::FindDefaultSkillRowForBall(
	const FName BallId,
	FPBBallSkillTableRow& OutRow) const
{
	OutRow = FPBBallSkillTableRow();

	FPBBallTableRow BallRow;
	if (!FindBallRow(BallId, BallRow) || BallRow.DefaultSkillIds.IsEmpty())
	{
		return false;
	}

	return FindSkillRow(BallRow.DefaultSkillIds[0], OutRow);
}

#pragma endregion

#pragma region Shop


void UPBTableDataSubsystem::GetAllShopRowName(TArray<FName>& OutShopIds)
{
	OutShopIds.Reset();
	
	if (!ShopTable)
	{
		return;
	}
	
	OutShopIds = ShopTable->GetRowNames();
}

bool UPBTableDataSubsystem::FindShopRow(FName RowName, FPBShopTableRow& OutRow) const
{
	return FindTableRow(ShopTable, RowName, OutRow, TEXT("FindShopRow"));
}

#pragma endregion

#pragma region Boss


bool UPBTableDataSubsystem::GetBossRowNames(TArray<FName>& OutRowNames) const
{
	OutRowNames.Reset();

	if (!IsValid(BossTable))
	{
		return false;
	}

	OutRowNames = BossTable->GetRowNames();
	return !OutRowNames.IsEmpty();
}

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

#pragma region Synergy

bool UPBTableDataSubsystem::GetAllSynergyRows(
	TArray<FName>& OutRowNames,
	TArray<FPBSynergyTableRow>& OutRows) const
{
	return GetAllTableRows(SynergyTable, OutRowNames, OutRows, TEXT("GetAllSynergyRows"));
}

bool UPBTableDataSubsystem::FindSynergyRow(const FName RowName, FPBSynergyTableRow& OutRow) const
{
	return FindTableRow(SynergyTable, RowName, OutRow, TEXT("FindSynergyRow"));
}

bool UPBTableDataSubsystem::FindSynergyTierRow(const FName RowName, FPBSynergyTierRow& OutRow) const
{
	return FindTableRow(SynergyTierTable, RowName, OutRow, TEXT("FindSynergyTierRow"));
}

bool UPBTableDataSubsystem::GetSynergyTierRows(
	const FName SynergyId,
	TArray<FPBSynergyTierRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(SynergyTierTable) || SynergyId.IsNone())
	{
		return false;
	}

	SynergyTierTable->ForeachRow<FPBSynergyTierRow>(
		TEXT("GetSynergyTierRows"),
		[SynergyId, &OutRows](const FName& RowName, const FPBSynergyTierRow& Row)
		{
			if (Row.SynergyId == SynergyId)
			{
				OutRows.Add(Row);
			}
		});

	OutRows.Sort(
		[](const FPBSynergyTierRow& Left, const FPBSynergyTierRow& Right)
		{
			return Left.RequiredCount < Right.RequiredCount;
		});

	return OutRows.Num() > 0;
}

bool UPBTableDataSubsystem::FindSynergyEffectRow(const FName RowName, FPBSynergyEffectRow& OutRow) const
{
	return FindTableRow(SynergyEffectTable, RowName, OutRow, TEXT("FindSynergyEffectRow"));
}

bool UPBTableDataSubsystem::GetSynergyEffectModifierRows(
	const FName SynergyEffectId,
	TArray<FPBSynergyEffectModifierRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(SynergyEffectModifierTable) || SynergyEffectId.IsNone())
	{
		return false;
	}

	SynergyEffectModifierTable->ForeachRow<FPBSynergyEffectModifierRow>(
		TEXT("GetSynergyEffectModifierRows"),
		[SynergyEffectId, &OutRows](const FName& RowName, const FPBSynergyEffectModifierRow& Row)
		{
			if (Row.SynergyEffectId == SynergyEffectId)
			{
				OutRows.Add(Row);
			}
		});

	return OutRows.Num() > 0;
}

bool UPBTableDataSubsystem::GetSynergyEffectTriggerRows(
	const FName SynergyEffectId,
	TArray<FPBSynergyEffectTriggerRow>& OutRows) const
{
	OutRows.Reset();
	if (!IsValid(SynergyEffectTriggerTable) || SynergyEffectId.IsNone())
	{
		return false;
	}

	SynergyEffectTriggerTable->ForeachRow<FPBSynergyEffectTriggerRow>(
		TEXT("GetSynergyEffectTriggerRows"),
		[SynergyEffectId, &OutRows](const FName& RowName, const FPBSynergyEffectTriggerRow& Row)
		{
			if (Row.SynergyEffectId == SynergyEffectId)
			{
				OutRows.Add(Row);
			}
		});

	return OutRows.Num() > 0;
}

#pragma endregion

#pragma region Relic
bool UPBTableDataSubsystem::FindRelicRow(FName RowName, FPBRelicTableRow& OutRow) const
{
	return FindTableRow(RelicTable,RowName,OutRow,TEXT("FindRelicRow"));
}

bool UPBTableDataSubsystem::FindRelicModifierRow(FName RowName, FPBRelicModifierRow OutRow) const
{
	return FindTableRow(RelicModifierTable,RowName,OutRow,TEXT("FindRelicModifierRow"));
}

void UPBTableDataSubsystem::GetRelicModifierRows(FName RelicId, TArray<FPBRelicModifierRow>& OutRows) const
{
	OutRows.Reset();

	if (!RelicModifierTable)
	{
		return;
	}

	TArray<FPBRelicModifierRow*> Rows;

	RelicModifierTable->GetAllRows<FPBRelicModifierRow>(
		TEXT("GetRelicModifierRows"),
		Rows);

	for (const FPBRelicModifierRow* Row : Rows)
	{
		if (!Row)
		{
			continue;
		}

		if (Row->RelicId != RelicId)
		{
			continue;
		}

		OutRows.Add(*Row);
	}
}

void UPBTableDataSubsystem::GetAllRelicIds(TArray<FName>& OutRelicIds) const
{
	OutRelicIds.Reset();

	if (!RelicTable)
	{
		return;
	}

	OutRelicIds = RelicTable->GetRowNames();
}
#pragma endregion 
