// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPlayerDataSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Bumper/Save/PBBumperLoadoutSaveGame.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"

namespace
{
	constexpr TCHAR BumperLoadoutSaveSlot[] = TEXT("PB_BumperLoadout");
	constexpr int32 BumperLoadoutUserIndex = 0;
}

void UPBPlayerDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPBTableDataSubsystem::StaticClass());

	const bool bLoadedBumperLoadout = LoadBumperLoadout();
	if (!bLoadedBumperLoadout)
	{
		InitializeDefaultBumpers();
	}

	bBumperPersistenceInitialized = true;
	if (!bLoadedBumperLoadout || bLoadedBumperLoadoutNeedsResave)
	{
		SaveBumperLoadout();
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBTableDataSubsystem* TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>())
		{
			TableDataSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
				this,
				&UPBPlayerDataSubsystem::HandleStartupGameDataLoaded);

			if (TableDataSubsystem->IsBumperTableReady())
			{
				SanitizeEquippedBumpers();
			}
		}
	}
}

void UPBPlayerDataSubsystem::Deinitialize()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBTableDataSubsystem* TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>())
		{
			TableDataSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
				this,
				&UPBPlayerDataSubsystem::HandleStartupGameDataLoaded);
		}
	}

	bBumperPersistenceInitialized = false;
	Super::Deinitialize();
}

void UPBPlayerDataSubsystem::AdvanceBossProgress(const int32 BossCount)
{
	if (CurrentBossIndex + 1 < BossCount)
	{
		++CurrentBossIndex;
	}
}

bool UPBPlayerDataSubsystem::EquipBumper(const EPBBumperSlotType SlotType, const FName BumperRowId)
{
	if (BumperRowId.IsNone())
	{
		return false;
	}

	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
	case EPBBumperSlotType::Side:
	case EPBBumperSlotType::Rebound:
	case EPBBumperSlotType::Special:
		return EquipBumperAtSlot(PBBumperEquipSlotUtils::GetDefaultEquipSlot(SlotType), BumperRowId);
	default:
		return false;
	}
}

bool UPBPlayerDataSubsystem::UnequipBumper(const EPBBumperSlotType SlotType)
{
	int32 RemovedCount = 0;
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::TopLeft);
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::TopRight);
		break;
	case EPBBumperSlotType::Side:
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::SideLeft);
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::SideRight);
		break;
	case EPBBumperSlotType::Rebound:
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::ReboundLeft);
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::ReboundRight);
		break;
	case EPBBumperSlotType::Special:
		RemovedCount += EquippedBumperRowIds.Remove(EPBBumperEquipSlot::Special);
		break;
	default:
		break;
	}

	if (RemovedCount > 0)
	{
		SaveBumperLoadout();
		return true;
	}

	return false;
}

bool UPBPlayerDataSubsystem::GetEquippedBumper(
	const EPBBumperSlotType SlotType,
	FName& OutBumperRowId) const
{
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
	case EPBBumperSlotType::Side:
	case EPBBumperSlotType::Rebound:
	case EPBBumperSlotType::Special:
		break;
	default:
		OutBumperRowId = NAME_None;
		return false;
	}

	const EPBBumperEquipSlot DefaultEquipSlot = PBBumperEquipSlotUtils::GetDefaultEquipSlot(SlotType);
	if (GetEquippedBumperAtSlot(DefaultEquipSlot, OutBumperRowId))
	{
		return true;
	}

	EPBBumperEquipSlot AlternateEquipSlot;
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		AlternateEquipSlot = EPBBumperEquipSlot::TopRight;
		break;
	case EPBBumperSlotType::Side:
		AlternateEquipSlot = EPBBumperEquipSlot::SideRight;
		break;
	case EPBBumperSlotType::Rebound:
		AlternateEquipSlot = EPBBumperEquipSlot::ReboundRight;
		break;
	default:
		OutBumperRowId = NAME_None;
		return false;
	}

	return GetEquippedBumperAtSlot(AlternateEquipSlot, OutBumperRowId);
}

bool UPBPlayerDataSubsystem::EquipBumperAtSlot(
	const EPBBumperEquipSlot EquipSlot,
	const FName BumperRowId)
{
	const FName NormalizedBumperRowId = PBBumperAssetIds::NormalizeBumperRowId(BumperRowId);
	EPBBumperSlotType SlotType;
	if (NormalizedBumperRowId.IsNone()
		|| !PBBumperEquipSlotUtils::TryGetSlotType(EquipSlot, SlotType))
	{
		return false;
	}

	if (IsBumperEquippedInAnotherSlot(NormalizedBumperRowId, EquipSlot))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[BumperEquip] Duplicate row rejected. TargetSlot=%s RowName=%s"),
			*UEnum::GetValueAsString(EquipSlot),
			*NormalizedBumperRowId.ToString());
		return false;
	}

	if (!ValidateBumperForSlot(EquipSlot, NormalizedBumperRowId))
	{
		return false;
	}

	EquippedBumperRowIds.Add(EquipSlot, NormalizedBumperRowId);
	SaveBumperLoadout();
	return true;
}

bool UPBPlayerDataSubsystem::UnequipBumperAtSlot(const EPBBumperEquipSlot EquipSlot)
{
	if (EquippedBumperRowIds.Remove(EquipSlot) > 0)
	{
		SaveBumperLoadout();
		return true;
	}

	return false;
}

bool UPBPlayerDataSubsystem::GetEquippedBumperAtSlot(
	const EPBBumperEquipSlot EquipSlot,
	FName& OutBumperRowId) const
{
	const FName* BumperRowId = EquippedBumperRowIds.Find(EquipSlot);
	if (!BumperRowId || BumperRowId->IsNone())
	{
		OutBumperRowId = NAME_None;
		return false;
	}

	OutBumperRowId = *BumperRowId;
	return true;
}

TArray<FPBEquippedBumperSlot> UPBPlayerDataSubsystem::GetEquippedBumperSlots() const
{
	static constexpr EPBBumperEquipSlot OrderedSlots[] =
	{
		EPBBumperEquipSlot::TopLeft,
		EPBBumperEquipSlot::TopRight,
		EPBBumperEquipSlot::SideLeft,
		EPBBumperEquipSlot::SideRight,
		EPBBumperEquipSlot::ReboundLeft,
		EPBBumperEquipSlot::ReboundRight,
		EPBBumperEquipSlot::Special
	};

	TArray<FPBEquippedBumperSlot> EquippedSlots;
	EquippedSlots.Reserve(UE_ARRAY_COUNT(OrderedSlots));
	for (const EPBBumperEquipSlot EquipSlot : OrderedSlots)
	{
		FName BumperRowId = NAME_None;
		if (GetEquippedBumperAtSlot(EquipSlot, BumperRowId))
		{
			FPBEquippedBumperSlot& EquippedSlot = EquippedSlots.AddDefaulted_GetRef();
			EquippedSlot.EquipSlot = EquipSlot;
			EquippedSlot.BumperRowId = BumperRowId;
		}
	}

	return EquippedSlots;
}

bool UPBPlayerDataSubsystem::IsBumperEquippedInAnotherSlot(
	const FName BumperRowId,
	const EPBBumperEquipSlot TargetSlot) const
{
	const FName NormalizedBumperRowId = PBBumperAssetIds::NormalizeBumperRowId(BumperRowId);
	if (NormalizedBumperRowId.IsNone())
	{
		return false;
	}

	for (const TPair<EPBBumperEquipSlot, FName>& EquippedBumper : EquippedBumperRowIds)
	{
		if (EquippedBumper.Key != TargetSlot
			&& PBBumperAssetIds::NormalizeBumperRowId(EquippedBumper.Value)
				== NormalizedBumperRowId)
		{
			return true;
		}
	}

	return false;
}

TArray<FName> UPBPlayerDataSubsystem::GetEquippedBumperRowIds() const
{
	TArray<FName> BumperRowIds;
	for (const FPBEquippedBumperSlot& EquippedSlot : GetEquippedBumperSlots())
	{
		BumperRowIds.AddUnique(EquippedSlot.BumperRowId);
	}

	return BumperRowIds;
}

void UPBPlayerDataSubsystem::InitializeDefaultBumpers()
{
	EquipBumperAtSlot(EPBBumperEquipSlot::ReboundLeft, PBBumperAssetIds::Bumper::Rebound_PowerPush);
	EquipBumperAtSlot(EPBBumperEquipSlot::ReboundRight, PBBumperAssetIds::Bumper::Rebound_KineticShell);
	EquipBumperAtSlot(EPBBumperEquipSlot::SideLeft, PBBumperAssetIds::Bumper::Side_ShieldCharge);
	EquipBumperAtSlot(EPBBumperEquipSlot::SideRight, PBBumperAssetIds::Bumper::Side_ManaCharge);
	EquipBumperAtSlot(EPBBumperEquipSlot::TopLeft, PBBumperAssetIds::Bumper::Top_ComboArc);
	EquipBumperAtSlot(EPBBumperEquipSlot::TopRight, PBBumperAssetIds::Bumper::Top_GroggyStrike);
	EquipBumperAtSlot(EPBBumperEquipSlot::Special, PBBumperAssetIds::Bumper::Gate_SpeedUp);
}

bool UPBPlayerDataSubsystem::LoadBumperLoadout()
{
	EquippedBumperRowIds.Reset();
	bLoadedBumperLoadoutNeedsResave = false;
	if (!UGameplayStatics::DoesSaveGameExist(BumperLoadoutSaveSlot, BumperLoadoutUserIndex))
	{
		return false;
	}

	const UPBBumperLoadoutSaveGame* SaveGame = Cast<UPBBumperLoadoutSaveGame>(
		UGameplayStatics::LoadGameFromSlot(BumperLoadoutSaveSlot, BumperLoadoutUserIndex));
	if (!IsValid(SaveGame) || SaveGame->SaveVersion != UPBBumperLoadoutSaveGame::CurrentSaveVersion)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperEquip] Loadout save is missing or has an unsupported version."));
		return false;
	}

	TSet<EPBBumperEquipSlot> LoadedSlots;
	TSet<FName> LoadedRows;
	for (const FPBEquippedBumperSlot& EquippedSlot : SaveGame->EquippedSlots)
	{
		EPBBumperSlotType SlotType;
		const FName NormalizedRowId = PBBumperAssetIds::NormalizeBumperRowId(EquippedSlot.BumperRowId);
		if (!PBBumperEquipSlotUtils::TryGetSlotType(EquippedSlot.EquipSlot, SlotType)
			|| NormalizedRowId.IsNone()
			|| LoadedSlots.Contains(EquippedSlot.EquipSlot)
			|| LoadedRows.Contains(NormalizedRowId))
		{
			bLoadedBumperLoadoutNeedsResave = true;
			continue;
		}

		EquippedBumperRowIds.Add(EquippedSlot.EquipSlot, NormalizedRowId);
		LoadedSlots.Add(EquippedSlot.EquipSlot);
		LoadedRows.Add(NormalizedRowId);
		bLoadedBumperLoadoutNeedsResave |= NormalizedRowId != EquippedSlot.BumperRowId;
	}

	return true;
}

bool UPBPlayerDataSubsystem::SaveBumperLoadout() const
{
	if (!bBumperPersistenceInitialized)
	{
		return true;
	}

	UPBBumperLoadoutSaveGame* SaveGame = Cast<UPBBumperLoadoutSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPBBumperLoadoutSaveGame::StaticClass()));
	if (!IsValid(SaveGame))
	{
		return false;
	}

	SaveGame->EquippedSlots = GetEquippedBumperSlots();
	const bool bSaved = UGameplayStatics::SaveGameToSlot(
		SaveGame,
		BumperLoadoutSaveSlot,
		BumperLoadoutUserIndex);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("[BumperEquip] Failed to save the bumper loadout."));
	}
	return bSaved;
}

bool UPBPlayerDataSubsystem::ValidateBumperForSlot(
	const EPBBumperEquipSlot EquipSlot,
	const FName BumperRowId) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableDataSubsystem) || !TableDataSubsystem->IsBumperTableReady())
	{
		// 에디터 단위 테스트와 초기 비동기 로딩 구간은 테이블 준비 후 일괄 정리합니다.
		return true;
	}

	FPBBumperTableRow BumperRow;
	if (!TableDataSubsystem->FindBumperRow(BumperRowId, BumperRow))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BumperEquip] Unknown bumper row rejected. Slot=%s RowName=%s"),
			*UEnum::GetValueAsString(EquipSlot),
			*BumperRowId.ToString());
		return false;
	}

	if (!PBBumperEquipSlotUtils::DoesBumperTypeMatchEquipSlot(BumperRow.BumperType, EquipSlot))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BumperEquip] Bumper type does not match slot. Slot=%s RowName=%s Type=%s"),
			*UEnum::GetValueAsString(EquipSlot),
			*BumperRowId.ToString(),
			*UEnum::GetValueAsString(BumperRow.BumperType));
		return false;
	}

	return true;
}

void UPBPlayerDataSubsystem::SanitizeEquippedBumpers()
{
	bool bChanged = false;
	TSet<FName> ValidRows;
	for (auto It = EquippedBumperRowIds.CreateIterator(); It; ++It)
	{
		const FName NormalizedRowId = PBBumperAssetIds::NormalizeBumperRowId(It.Value());
		if (NormalizedRowId.IsNone()
			|| ValidRows.Contains(NormalizedRowId)
			|| !ValidateBumperForSlot(It.Key(), NormalizedRowId))
		{
			It.RemoveCurrent();
			bChanged = true;
			continue;
		}

		if (It.Value() != NormalizedRowId)
		{
			It.Value() = NormalizedRowId;
			bChanged = true;
		}
		ValidRows.Add(NormalizedRowId);
	}

	if (bChanged)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperEquip] Invalid saved loadout entries were removed."));
		SaveBumperLoadout();
	}
}

void UPBPlayerDataSubsystem::HandleStartupGameDataLoaded()
{
	SanitizeEquippedBumpers();
}
