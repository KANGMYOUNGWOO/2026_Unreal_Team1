// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPlayerDataSubsystem.h"

#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"

void UPBPlayerDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultBumpersForTest();
}

bool UPBPlayerDataSubsystem::EquipBumper(const EPBBumperSlotType SlotType, const FName BumperRowId)
{
	if (BumperRowId.IsNone())
	{
		return false;
	}

	EquippedBumperRowIds.Add(SlotType, BumperRowId);
	return true;
}

bool UPBPlayerDataSubsystem::UnequipBumper(const EPBBumperSlotType SlotType)
{
	return EquippedBumperRowIds.Remove(SlotType) > 0;
}

bool UPBPlayerDataSubsystem::GetEquippedBumper(
	const EPBBumperSlotType SlotType,
	FName& OutBumperRowId) const
{
	const FName* BumperRowId = EquippedBumperRowIds.Find(SlotType);
	if (!BumperRowId || BumperRowId->IsNone())
	{
		OutBumperRowId = NAME_None;
		return false;
	}

	OutBumperRowId = *BumperRowId;
	return true;
}

TArray<FName> UPBPlayerDataSubsystem::GetEquippedBumperRowIds() const
{
	TArray<FName> BumperRowIds;

	FName BumperRowId = NAME_None;
	if (GetEquippedBumper(EPBBumperSlotType::Rebound, BumperRowId))
	{
		BumperRowIds.Add(BumperRowId);
	}

	if (GetEquippedBumper(EPBBumperSlotType::Side, BumperRowId))
	{
		BumperRowIds.Add(BumperRowId);
	}

	if (GetEquippedBumper(EPBBumperSlotType::Top, BumperRowId))
	{
		BumperRowIds.Add(BumperRowId);
	}

	if (GetEquippedBumper(EPBBumperSlotType::Special, BumperRowId))
	{
		BumperRowIds.Add(BumperRowId);
	}

	return BumperRowIds;
}

void UPBPlayerDataSubsystem::InitializeDefaultBumpersForTest()
{
	EquipBumper(EPBBumperSlotType::Rebound, PBBumperAssetIds::Bumper::Test01);
	EquipBumper(EPBBumperSlotType::Side, PBBumperAssetIds::Bumper::Test02);
	EquipBumper(EPBBumperSlotType::Top, PBBumperAssetIds::Bumper::Test03);
	EquipBumper(EPBBumperSlotType::Special, PBBumperAssetIds::Bumper::Gate01);
}
