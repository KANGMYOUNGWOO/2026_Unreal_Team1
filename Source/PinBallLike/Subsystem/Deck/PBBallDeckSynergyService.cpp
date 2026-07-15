#include "PBBallDeckSynergyService.h"

#include "PBBallDeckSubsystem.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/Struct/Deck/PBDeckOwnedBallData.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"

void UPBBallDeckSynergyService::Initialize(UPBBallDeckSubsystem* InDeckSubsystem)
{
	DeckSubsystem = InDeckSubsystem;
	ActiveSynergyStates.Reset();
	BindDeckEvents();
	RefreshSynergyStatesFromPlacedDeck();
}

void UPBBallDeckSynergyService::Deinitialize()
{
	UnbindDeckEvents();
	ActiveSynergyStates.Reset();
	DeckSubsystem = nullptr;
}

void UPBBallDeckSynergyService::RefreshSynergyStatesFromPlacedDeck()
{
	TArray<FPBSynergyState> NewStates;
	if (DeckSubsystem)
	{
		BuildSynergyStatesFromBallInstanceIds(DeckSubsystem->GetAllPlacedBallInstanceIds(), NewStates);
	}

	if (ActiveSynergyStates == NewStates)
	{
		return;
	}

	ActiveSynergyStates = MoveTemp(NewStates);
	OnSynergyStatesChanged.Broadcast(ActiveSynergyStates);
}

bool UPBBallDeckSynergyService::BuildSynergyStatesFromBallIds(
	const TArray<FName>& BallIds,
	TArray<FPBSynergyState>& OutStates) const
{
	OutStates.Reset();

	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem)
	{
		return false;
	}

	TMap<FName, int32> SynergyCounts;
	TSet<FName> CountedBallIds;
	for (const FName BallId : BallIds)
	{
		if (BallId.IsNone() || CountedBallIds.Contains(BallId))
		{
			continue;
		}
		CountedBallIds.Add(BallId);

		FPBBallTableRow BallRow;
		if (!TableDataSubsystem->FindBallRow(BallId, BallRow))
		{
			continue;
		}

		for (const EPBBallRaceType RaceType : BallRow.RaceTypes)
		{
			const FName RaceSynergyId = GetRaceSynergyId(RaceType);
			if (!RaceSynergyId.IsNone())
			{
				SynergyCounts.FindOrAdd(RaceSynergyId)++;
			}
		}

		const FName ClassSynergyId = GetClassSynergyId(BallRow.ClassType);
		if (!ClassSynergyId.IsNone())
		{
			SynergyCounts.FindOrAdd(ClassSynergyId)++;
		}
	}

	TArray<FName> SynergyRowNames;
	TArray<FPBSynergyTableRow> SynergyRows;
	if (!TableDataSubsystem->GetAllSynergyRows(SynergyRowNames, SynergyRows))
	{
		return false;
	}

	OutStates.Reserve(SynergyRows.Num());
	for (int32 Index = 0; Index < SynergyRows.Num(); ++Index)
	{
		if (!SynergyRowNames.IsValidIndex(Index))
		{
			continue;
		}

		const FName SynergyId = SynergyRowNames[Index];
		const int32* CurrentCount = SynergyCounts.Find(SynergyId);
		if (!CurrentCount || *CurrentCount <= 0)
		{
			continue;
		}

		TArray<FPBSynergyTierRow> TierRows;
		if (!TableDataSubsystem->GetSynergyTierRows(SynergyId, TierRows))
		{
			continue;
		}

		FPBSynergyState State;
		State.SynergyId = SynergyId;
		State.CurrentCount = *CurrentCount;
		for (const FPBSynergyTierRow& TierRow : TierRows)
		{
			if (*CurrentCount >= TierRow.RequiredCount && TierRow.RequiredCount >= State.ActiveRequiredCount)
			{
				State.ActiveRequiredCount = TierRow.RequiredCount;
				State.ActiveSynergyEffectId = TierRow.SynergyEffectId;
			}
		}

		OutStates.Add(State);
	}

	return true;
}

bool UPBBallDeckSynergyService::BuildSynergyStatesFromBallInstanceIds(
	const TArray<int32>& BallInstanceIds,
	TArray<FPBSynergyState>& OutStates) const
{
	OutStates.Reset();
	if (!DeckSubsystem)
	{
		return false;
	}

	TArray<FName> BallIds;
	BallIds.Reserve(BallInstanceIds.Num());
	for (const int32 BallInstanceId : BallInstanceIds)
	{
		const FPBDeckOwnedBallData* OwnedBallData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
		if (OwnedBallData && OwnedBallData->IsValid())
		{
			BallIds.Add(OwnedBallData->BallId);
		}
	}

	return BuildSynergyStatesFromBallIds(BallIds, OutStates);
}

void UPBBallDeckSynergyService::BindDeckEvents()
{
	if (bDeckEventsBound || !DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.AddUniqueDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddUniqueDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeckChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.AddUniqueDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeckChanged);
	DeckSubsystem->OnBenchSlotChanged.AddUniqueDynamic(
		this,
		&UPBBallDeckSynergyService::HandleBenchSlotChanged);
	DeckSubsystem->OnBenchSlotsSwapped.AddUniqueDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeckChanged);
	DeckSubsystem->OnBallSold.AddUniqueDynamic(
		this,
		&UPBBallDeckSynergyService::HandleBallSold);

	bDeckEventsBound = true;
}

void UPBBallDeckSynergyService::UnbindDeckEvents()
{
	if (!bDeckEventsBound || !DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeckChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeckChanged);
	DeckSubsystem->OnBenchSlotChanged.RemoveDynamic(
		this,
		&UPBBallDeckSynergyService::HandleBenchSlotChanged);
	DeckSubsystem->OnBenchSlotsSwapped.RemoveDynamic(
		this,
		&UPBBallDeckSynergyService::HandleDeckChanged);
	DeckSubsystem->OnBallSold.RemoveDynamic(
		this,
		&UPBBallDeckSynergyService::HandleBallSold);

	bDeckEventsBound = false;
}

const UPBTableDataSubsystem* UPBBallDeckSynergyService::GetTableDataSubsystem() const
{
	const UGameInstance* GameInstance = DeckSubsystem ? DeckSubsystem->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
}

void UPBBallDeckSynergyService::HandleDeploymentSlotChanged(const int32 SlotIndex, const int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	RefreshSynergyStatesFromPlacedDeck();
}

void UPBBallDeckSynergyService::HandleDeckChanged()
{
	RefreshSynergyStatesFromPlacedDeck();
}

void UPBBallDeckSynergyService::HandleBenchSlotChanged(const int32 SlotIndex, const int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	RefreshSynergyStatesFromPlacedDeck();
}

void UPBBallDeckSynergyService::HandleBallSold(const int32 BallInstanceId, const FName BallId, const int32 SellPrice)
{
	(void)BallInstanceId;
	(void)BallId;
	(void)SellPrice;
	RefreshSynergyStatesFromPlacedDeck();
}

FName UPBBallDeckSynergyService::GetRaceSynergyId(const EPBBallRaceType RaceType)
{
	const UEnum* Enum = StaticEnum<EPBBallRaceType>();
	return Enum ? FName(*Enum->GetNameStringByValue(static_cast<int64>(RaceType))) : NAME_None;
}

FName UPBBallDeckSynergyService::GetClassSynergyId(const EPBBallClassType ClassType)
{
	const UEnum* Enum = StaticEnum<EPBBallClassType>();
	return Enum ? FName(*Enum->GetNameStringByValue(static_cast<int64>(ClassType))) : NAME_None;
}
