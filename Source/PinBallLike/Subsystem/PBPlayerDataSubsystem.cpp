// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPlayerDataSubsystem.h"

#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"

void UPBPlayerDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultBumpersForTest();
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

	return RemovedCount > 0;
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

	EquippedBumperRowIds.Add(EquipSlot, NormalizedBumperRowId);
	return true;
}

bool UPBPlayerDataSubsystem::UnequipBumperAtSlot(const EPBBumperEquipSlot EquipSlot)
{
	return EquippedBumperRowIds.Remove(EquipSlot) > 0;
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

void UPBPlayerDataSubsystem::InitializeDefaultBumpersForTest()
{
	EquipBumperAtSlot(EPBBumperEquipSlot::ReboundLeft, PBBumperAssetIds::Bumper::Rebound_PowerPush);
	EquipBumperAtSlot(EPBBumperEquipSlot::ReboundRight, PBBumperAssetIds::Bumper::Rebound_KineticShell);
	EquipBumperAtSlot(EPBBumperEquipSlot::SideLeft, PBBumperAssetIds::Bumper::Side_ShieldCharge);
	EquipBumperAtSlot(EPBBumperEquipSlot::SideRight, PBBumperAssetIds::Bumper::Side_ManaCharge);
	EquipBumperAtSlot(EPBBumperEquipSlot::TopLeft, PBBumperAssetIds::Bumper::Top_ComboArc);
	EquipBumperAtSlot(EPBBumperEquipSlot::TopRight, PBBumperAssetIds::Bumper::Top_GroggyStrike);
	EquipBumperAtSlot(EPBBumperEquipSlot::Special, PBBumperAssetIds::Bumper::Gate_SpeedUp);
}
