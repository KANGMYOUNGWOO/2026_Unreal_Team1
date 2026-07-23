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
	InitialGold = Gold;

	const bool bLoadedBumperLoadout = LoadBumperLoadout();
	if (!bLoadedBumperLoadout)
	{
		InitializeDefaultBumpers();
	}

	bBumperPersistenceInitialized = true;
	if (!bLoadedBumperLoadout || bLoadedBumperLoadoutNeedsResave)
	{
		SaveBumperLoadout(EquippedBumperRowIds);
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

bool UPBPlayerDataSubsystem::SetCurrentBossIndex(const int32 NewBossIndex, const int32 BossCount)
{
	if (NewBossIndex < 0 || NewBossIndex >= BossCount)
	{
		return false;
	}

	CurrentBossIndex = NewBossIndex;
	return true;
}

void UPBPlayerDataSubsystem::AdvanceBossProgress(const int32 BossCount)
{
	if (CurrentBossIndex + 1 < BossCount)
	{
		++CurrentBossIndex;
	}
}

void UPBPlayerDataSubsystem::ResetRunData()
{
	const int32 PreviousGold = Gold;
	Gold = FMath::Max(InitialGold, 0);
	if (Gold != PreviousGold)
	{
		OnGoldChanged.Broadcast(Gold);
	}

	CurrentBossIndex = 0;
	SetCurrentBossIndex(0, 4);

	UE_LOG(LogTemp, Log, TEXT("[PlayerData] Run data reset. Gold=%d BossIndex=%d"),
		Gold,
		CurrentBossIndex);
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
	TMap<EPBBumperEquipSlot, FName> CandidateLoadout = EquippedBumperRowIds;
	int32 RemovedCount = 0;
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::TopLeft);
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::TopRight);
		break;
	case EPBBumperSlotType::Side:
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::SideLeft);
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::SideRight);
		break;
	case EPBBumperSlotType::Rebound:
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::ReboundLeft);
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::ReboundRight);
		break;
	case EPBBumperSlotType::Special:
		RemovedCount += CandidateLoadout.Remove(EPBBumperEquipSlot::Special);
		break;
	default:
		break;
	}

	if (RemovedCount > 0)
	{
		return CommitBumperLoadout(MoveTemp(CandidateLoadout));
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

	TMap<EPBBumperEquipSlot, FName> CandidateLoadout = EquippedBumperRowIds;
	CandidateLoadout.Add(EquipSlot, NormalizedBumperRowId);
	return CommitBumperLoadout(MoveTemp(CandidateLoadout));
}

bool UPBPlayerDataSubsystem::UnequipBumperAtSlot(const EPBBumperEquipSlot EquipSlot)
{
	TMap<EPBBumperEquipSlot, FName> CandidateLoadout = EquippedBumperRowIds;
	if (CandidateLoadout.Remove(EquipSlot) > 0)
	{
		return CommitBumperLoadout(MoveTemp(CandidateLoadout));
	}

	return false;
}

bool UPBPlayerDataSubsystem::MoveEquippedBumperBetweenSlots(
	const EPBBumperEquipSlot SourceSlot,
	const EPBBumperEquipSlot TargetSlot)
{
	FName SourceRowId = NAME_None;
	if (!GetEquippedBumperAtSlot(SourceSlot, SourceRowId))
	{
		return false;
	}

	if (SourceSlot == TargetSlot)
	{
		return true;
	}

	EPBBumperSlotType SourceSlotType;
	EPBBumperSlotType TargetSlotType;
	if (!PBBumperEquipSlotUtils::TryGetSlotType(SourceSlot, SourceSlotType)
		|| !PBBumperEquipSlotUtils::TryGetSlotType(TargetSlot, TargetSlotType)
		|| SourceSlotType != TargetSlotType)
	{
		return false;
	}

	SourceRowId = PBBumperAssetIds::NormalizeBumperRowId(SourceRowId);
	if (SourceRowId.IsNone() || !ValidateBumperForSlot(TargetSlot, SourceRowId))
	{
		return false;
	}

	FName TargetRowId = NAME_None;
	const bool bHasTargetRow = GetEquippedBumperAtSlot(TargetSlot, TargetRowId);
	if (bHasTargetRow)
	{
		TargetRowId = PBBumperAssetIds::NormalizeBumperRowId(TargetRowId);
		if (TargetRowId.IsNone()
			|| TargetRowId == SourceRowId
			|| !ValidateBumperForSlot(SourceSlot, TargetRowId))
		{
			return false;
		}
	}

	TMap<EPBBumperEquipSlot, FName> CandidateLoadout = EquippedBumperRowIds;
	CandidateLoadout.Add(TargetSlot, SourceRowId);
	if (bHasTargetRow)
	{
		CandidateLoadout.Add(SourceSlot, TargetRowId);
	}
	else
	{
		CandidateLoadout.Remove(SourceSlot);
	}

	return CommitBumperLoadout(MoveTemp(CandidateLoadout));
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
	return BuildEquippedBumperSlots(EquippedBumperRowIds);
}

TArray<FPBEquippedBumperSlot> UPBPlayerDataSubsystem::BuildEquippedBumperSlots(
	const TMap<EPBBumperEquipSlot, FName>& Loadout)
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
		const FName* BumperRowId = Loadout.Find(EquipSlot);
		if (BumperRowId && !BumperRowId->IsNone())
		{
			FPBEquippedBumperSlot& EquippedSlot = EquippedSlots.AddDefaulted_GetRef();
			EquippedSlot.EquipSlot = EquipSlot;
			EquippedSlot.BumperRowId = *BumperRowId;
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

void UPBPlayerDataSubsystem::GainGold(int32 Amount)
{
	const int32 PreviousGold = Gold;
	Gold = FMath::Max(Gold + Amount, 0);
	if (Gold != PreviousGold)
	{
		OnGoldChanged.Broadcast(Gold);
	}
}

void UPBPlayerDataSubsystem::SpendGold(int32 Amount)
{
	if (Gold < Amount)
	{
		return;
	}

	const int32 PreviousGold = Gold;
	Gold -= Amount;
	if (Gold != PreviousGold)
	{
		OnGoldChanged.Broadcast(Gold);
	}
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

bool UPBPlayerDataSubsystem::CommitBumperLoadout(
	TMap<EPBBumperEquipSlot, FName>&& CandidateLoadout)
{
	bool bLoadoutsMatch = CandidateLoadout.Num() == EquippedBumperRowIds.Num();
	if (bLoadoutsMatch)
	{
		for (const TPair<EPBBumperEquipSlot, FName>& EquippedBumper : EquippedBumperRowIds)
		{
			const FName* CandidateRowId = CandidateLoadout.Find(EquippedBumper.Key);
			if (!CandidateRowId || *CandidateRowId != EquippedBumper.Value)
			{
				bLoadoutsMatch = false;
				break;
			}
		}
	}

	if (bLoadoutsMatch)
	{
		return true;
	}

	if (!SaveBumperLoadout(CandidateLoadout))
	{
		return false;
	}

	EquippedBumperRowIds = MoveTemp(CandidateLoadout);
	return true;
}

bool UPBPlayerDataSubsystem::SaveBumperLoadout(
	const TMap<EPBBumperEquipSlot, FName>& Loadout) const
{
	if (!bBumperPersistenceInitialized)
	{
		return true;
	}

	return WriteBumperLoadout(BuildEquippedBumperSlots(Loadout));
}

bool UPBPlayerDataSubsystem::WriteBumperLoadout(
	const TArray<FPBEquippedBumperSlot>& EquippedSlots) const
{
#if WITH_DEV_AUTOMATION_TESTS
	if (BumperLoadoutWriterOverride)
	{
		return BumperLoadoutWriterOverride(EquippedSlots);
	}
#endif

	UPBBumperLoadoutSaveGame* SaveGame = Cast<UPBBumperLoadoutSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPBBumperLoadoutSaveGame::StaticClass()));
	if (!IsValid(SaveGame))
	{
		return false;
	}

	SaveGame->EquippedSlots = EquippedSlots;
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
	TMap<EPBBumperEquipSlot, FName> SanitizedLoadout = EquippedBumperRowIds;
	bool bChanged = false;
	TSet<FName> ValidRows;
	for (auto It = SanitizedLoadout.CreateIterator(); It; ++It)
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
		EquippedBumperRowIds = MoveTemp(SanitizedLoadout);
		UE_LOG(LogTemp, Warning, TEXT("[BumperEquip] Invalid saved loadout entries were removed."));
		SaveBumperLoadout(EquippedBumperRowIds);
	}
}

void UPBPlayerDataSubsystem::HandleStartupGameDataLoaded()
{
	SanitizeEquippedBumpers();
}
