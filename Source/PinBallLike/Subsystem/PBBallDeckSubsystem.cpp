// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckSubsystem.h"

void UPBBallDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDeploymentSlots();
	InitializeBenchSlots();
}

int32 UPBBallDeckSubsystem::AddOwnedBall(int32 BallId, int32 StarLevel)
{
	if (BallId == 0)
	{
		return INDEX_NONE;
	}

	const int32 NewInstanceId = NextBallInstanceId++;
	FPBBallInstanceData& NewBallData = OwnedBallDataMap.Add(NewInstanceId);
	NewBallData.InstanceId = NewInstanceId;
	NewBallData.BallId = BallId;
	NewBallData.StarLevel = FMath::Max(StarLevel, 1);

	return NewInstanceId;
}

const FPBBallInstanceData* UPBBallDeckSubsystem::GetOwnedBallData(int32 BallInstanceId) const
{
	return OwnedBallDataMap.Find(BallInstanceId);
}

bool UPBBallDeckSubsystem::HasOwnedBall(int32 BallInstanceId) const
{
	return OwnedBallDataMap.Contains(BallInstanceId);
}

#pragma region Deployment Slot

EPBBallPartyRole UPBBallDeckSubsystem::GetDeploymentRole(int32 SlotIndex) const
{
	if (!IsDeploymentSlotValid(SlotIndex))
	{
		return EPBBallPartyRole::None;
	}

	return SlotIndex == 0
		? EPBBallPartyRole::Leader
		: EPBBallPartyRole::Follower;
}

bool UPBBallDeckSubsystem::SetDeploymentSlot(int32 SlotIndex, int32 BallInstanceId)
{
	if (!IsDeploymentSlotValid(SlotIndex) || !HasOwnedBall(BallInstanceId))
	{
		return false;
	}

	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();

	ClearBallInstanceFromSlots(BallInstanceId);
	DeploymentSlots[SlotIndex].BallInstanceId = BallInstanceId;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	return true;
}

bool UPBBallDeckSubsystem::ClearDeploymentSlot(int32 SlotIndex)
{
	if (!IsDeploymentSlotValid(SlotIndex))
	{
		return false;
	}

	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();

	DeploymentSlots[SlotIndex].BallInstanceId = INDEX_NONE;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	return true;
}

bool UPBBallDeckSubsystem::SwapDeploymentSlots(int32 FirstIndex, int32 SecondIndex)
{
	if (!IsDeploymentSlotValid(FirstIndex) || !IsDeploymentSlotValid(SecondIndex))
	{
		return false;
	}

	if (FirstIndex == SecondIndex)
	{
		return true;
	}

	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();

	Swap(DeploymentSlots[FirstIndex].BallInstanceId, DeploymentSlots[SecondIndex].BallInstanceId);
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	return true;
}

bool UPBBallDeckSubsystem::IsDeploymentSlotValid(int32 SlotIndex) const
{
	return DeploymentSlots.IsValidIndex(SlotIndex);
}

bool UPBBallDeckSubsystem::IsDeploymentSlotOccupied(int32 SlotIndex) const
{
	return IsDeploymentSlotValid(SlotIndex) && DeploymentSlots[SlotIndex].BallInstanceId != INDEX_NONE;
}

int32 UPBBallDeckSubsystem::GetDeploymentSlotBallInstanceId(int32 SlotIndex) const
{
	return IsDeploymentSlotValid(SlotIndex) ? DeploymentSlots[SlotIndex].BallInstanceId : INDEX_NONE;
}

int32 UPBBallDeckSubsystem::GetLeaderBallInstanceId() const
{
	return GetDeploymentSlotBallInstanceId(0);
}

TArray<int32> UPBBallDeckSubsystem::GetDeploymentBallInstanceIds() const
{
	TArray<int32> BallInstanceIds;
	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.BallInstanceId != INDEX_NONE)
		{
			BallInstanceIds.Add(DeploymentSlot.BallInstanceId);	
		}
	}
	return BallInstanceIds;
}

TArray<int32> UPBBallDeckSubsystem::GetFollowerBallInstanceIds() const
{
	TArray<int32> BallInstanceIds;
	for (int32 SlotIndex = 1; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		if (DeploymentSlots[SlotIndex].BallInstanceId != INDEX_NONE)
		{
			BallInstanceIds.Add(DeploymentSlots[SlotIndex].BallInstanceId);
		}
	}
	return BallInstanceIds;
}

int32 UPBBallDeckSubsystem::GetDeploymentBallCount() const
{
	int32 BallCount = 0;
	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.BallInstanceId != INDEX_NONE)
		{
			++BallCount;
		}
	}
	return BallCount;
}

bool UPBBallDeckSubsystem::HasLeaderBall() const
{
	return GetLeaderBallInstanceId() != INDEX_NONE;
}

bool UPBBallDeckSubsystem::CanBuildDeploymentParty() const
{
	return HasLeaderBall() && GetDeploymentBallCount() > 0;
}

void UPBBallDeckSubsystem::CompactDeploymentSlots()
{
	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
}

bool UPBBallDeckSubsystem::RotateDeploymentSlots()
{
	if (GetDeploymentBallCount() < 2)
	{
		return false;
	}

	CompactDeploymentSlotsInternal();

	const int32 PreviousLeaderBallInstanceId = DeploymentSlots[0].BallInstanceId;
	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num() - 1; ++SlotIndex)
	{
		DeploymentSlots[SlotIndex].BallInstanceId = DeploymentSlots[SlotIndex + 1].BallInstanceId;
	}
	DeploymentSlots.Last().BallInstanceId = PreviousLeaderBallInstanceId;

	CompactDeploymentSlotsInternal();
	OnDeploymentSlotsRotated.Broadcast();
	return true;
}

TArray<int32> UPBBallDeckSubsystem::CaptureDeploymentSlotBallInstanceIds() const
{
	TArray<int32> BallInstanceIds;
	BallInstanceIds.Reserve(DeploymentSlots.Num());
	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		BallInstanceIds.Add(DeploymentSlot.BallInstanceId);
	}
	return BallInstanceIds;
}

bool UPBBallDeckSubsystem::CompactDeploymentSlotsInternal()
{
	bool bChanged = false;
	TArray<int32> OrderedBallInstanceIds;
	OrderedBallInstanceIds.Reserve(MaxDeploymentSlotCount);

	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.BallInstanceId != INDEX_NONE)
		{
			OrderedBallInstanceIds.Add(DeploymentSlot.BallInstanceId);
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		const int32 PreviousBallInstanceId = DeploymentSlots[SlotIndex].BallInstanceId;
		DeploymentSlots[SlotIndex].BallInstanceId = OrderedBallInstanceIds.IsValidIndex(SlotIndex)
			? OrderedBallInstanceIds[SlotIndex]
			: INDEX_NONE;

		if (PreviousBallInstanceId != DeploymentSlots[SlotIndex].BallInstanceId)
		{
			bChanged = true;
		}
	}

	return bChanged;
}

void UPBBallDeckSubsystem::BroadcastDeploymentSlotChange(const TArray<int32>& PreviousBallInstanceIds)
{
	int32 ChangedSlotIndex = INDEX_NONE;
	int32 ChangedBallInstanceId = INDEX_NONE;
	int32 ChangedSlotCount = 0;

	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		const int32 PreviousBallInstanceId = PreviousBallInstanceIds.IsValidIndex(SlotIndex)
			? PreviousBallInstanceIds[SlotIndex]
			: INDEX_NONE;
		const int32 CurrentBallInstanceId = DeploymentSlots[SlotIndex].BallInstanceId;
		if (PreviousBallInstanceId == CurrentBallInstanceId)
		{
			continue;
		}

		ChangedSlotIndex = SlotIndex;
		ChangedBallInstanceId = CurrentBallInstanceId;
		++ChangedSlotCount;
	}

	if (ChangedSlotCount == 1)
	{
		OnDeploymentSlotChanged.Broadcast(ChangedSlotIndex, ChangedBallInstanceId);
	}
	else if (ChangedSlotCount > 1)
	{
		OnDeploymentSlotsReordered.Broadcast();
	}
}

void UPBBallDeckSubsystem::ClearBallInstanceFromSlots(int32 BallInstanceId)
{
	if (BallInstanceId == INDEX_NONE)
	{
		return;
	}

	for (FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.BallInstanceId == BallInstanceId)
		{
			DeploymentSlot.BallInstanceId = INDEX_NONE;
		}
	}

	for (FPBBallDeckSlot& BenchSlot : BenchSlots)
	{
		if (BenchSlot.BallInstanceId == BallInstanceId)
		{
			BenchSlot.BallInstanceId = INDEX_NONE;
		}
	}
}

void UPBBallDeckSubsystem::InitializeDeploymentSlots()
{
	DeploymentSlots.SetNum(MaxDeploymentSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		DeploymentSlots[SlotIndex].SlotIndex = SlotIndex;
		DeploymentSlots[SlotIndex].SlotType = EPBBallDeckSlotType::Deployment;
		DeploymentSlots[SlotIndex].BallInstanceId = INDEX_NONE;
	}
}

#pragma endregion

#pragma region Bench

void UPBBallDeckSubsystem::InitializeBenchSlots()
{
	BenchSlots.SetNum(MaxBenchSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < BenchSlots.Num(); ++SlotIndex)
	{
		BenchSlots[SlotIndex].SlotIndex = SlotIndex;
		BenchSlots[SlotIndex].SlotType = EPBBallDeckSlotType::Bench;
		BenchSlots[SlotIndex].BallInstanceId = INDEX_NONE;
	}
}

bool UPBBallDeckSubsystem::SetBenchSlot(int32 SlotIndex, int32 BallInstanceId)
{
	if (!IsBenchSlotValid(SlotIndex) || !HasOwnedBall(BallInstanceId))
	{
		return false;
	}

	ClearBallInstanceFromSlots(BallInstanceId);
	BenchSlots[SlotIndex].BallInstanceId = BallInstanceId;
	OnBenchSlotChanged.Broadcast(SlotIndex, BallInstanceId);
	return true;
}

bool UPBBallDeckSubsystem::ClearBenchSlot(int32 SlotIndex)
{
	if (!IsBenchSlotValid(SlotIndex))
	{
		return false;
	}
	BenchSlots[SlotIndex].BallInstanceId = INDEX_NONE;
	OnBenchSlotChanged.Broadcast(SlotIndex, INDEX_NONE);
	return true;
}

bool UPBBallDeckSubsystem::SwapBenchSlots(int32 FirstIndex, int32 SecondIndex)
{
	if (!IsBenchSlotValid(FirstIndex) || !IsBenchSlotValid(SecondIndex))
	{
		return false;
	}

	if (FirstIndex == SecondIndex)
	{
		return true;
	}

	Swap(BenchSlots[FirstIndex].BallInstanceId, BenchSlots[SecondIndex].BallInstanceId);
	OnBenchSlotsSwapped.Broadcast();
	return true;
}

bool UPBBallDeckSubsystem::IsBenchSlotValid(int32 SlotIndex) const
{
	return BenchSlots.IsValidIndex(SlotIndex);
}

bool UPBBallDeckSubsystem::IsBenchSlotOccupied(int32 SlotIndex) const
{
	return IsBenchSlotValid(SlotIndex) && BenchSlots[SlotIndex].BallInstanceId != INDEX_NONE;
}

bool UPBBallDeckSubsystem::HasEmptyBenchSlot() const
{
	return FindEmptyBenchSlot() != INDEX_NONE;
}

int32 UPBBallDeckSubsystem::FindEmptyBenchSlot() const
{
	for (int32 SlotIndex = 0; SlotIndex < BenchSlots.Num(); ++SlotIndex)
	{
		if (BenchSlots[SlotIndex].BallInstanceId == INDEX_NONE)
		{
			return SlotIndex;
		}
	}
	return INDEX_NONE;
}

bool UPBBallDeckSubsystem::AddBenchBall(int32 BallInstanceId)
{
	if (!HasOwnedBall(BallInstanceId))
	{
		return false;
	}

	const int32 EmptySlotIndex = FindEmptyBenchSlot();
	if (EmptySlotIndex == INDEX_NONE)
	{
		return false;
	}

	return SetBenchSlot(EmptySlotIndex, BallInstanceId);
}

bool UPBBallDeckSubsystem::AddNewBallToBench(int32 BallId, int32 StarLevel)
{
	if (!HasEmptyBenchSlot())
	{
		return false;
	}

	const int32 NewBallInstanceId = AddOwnedBall(BallId, StarLevel);
	return NewBallInstanceId != INDEX_NONE && AddBenchBall(NewBallInstanceId);
}

int32 UPBBallDeckSubsystem::GetBenchBallInstanceId(int32 SlotIndex) const
{
	return IsBenchSlotValid(SlotIndex) ? BenchSlots[SlotIndex].BallInstanceId : INDEX_NONE;
}

TArray<int32> UPBBallDeckSubsystem::GetBenchBallInstanceIds() const
{
	TArray<int32> BallInstanceIds;
	for (const FPBBallDeckSlot& BenchSlot : BenchSlots)
	{
		if (BenchSlot.BallInstanceId != INDEX_NONE)
		{
			BallInstanceIds.Add(BenchSlot.BallInstanceId);
		}
	}
	return BallInstanceIds;
}

int32 UPBBallDeckSubsystem::GetBenchBallCount() const
{
	int32 BallCount = 0;
	for (const FPBBallDeckSlot& BenchSlot : BenchSlots)
	{
		if (BenchSlot.BallInstanceId != INDEX_NONE)
		{
			++BallCount;
		}
	}
	return BallCount;
}


#pragma endregion
