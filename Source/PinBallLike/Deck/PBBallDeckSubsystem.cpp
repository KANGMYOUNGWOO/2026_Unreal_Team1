// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckSubsystem.h"

void UPBBallDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDeploymentSlots();
}

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

bool UPBBallDeckSubsystem::SetDeploymentSlot(int32 SlotIndex, APBBallBase* Ball)
{
	if (!IsDeploymentSlotValid(SlotIndex) || Ball == nullptr)
	{
		return false;
	}

	const TArray<APBBallBase*> PreviousBalls = CaptureDeploymentSlotBalls();

	for (FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.Ball == Ball)
		{
			DeploymentSlot.Ball = nullptr;
		}
	}

	DeploymentSlots[SlotIndex].Ball = Ball;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBalls);
	return true;
}

bool UPBBallDeckSubsystem::ClearDeploymentSlot(int32 SlotIndex)
{
	if (!IsDeploymentSlotValid(SlotIndex))
	{
		return false;
	}

	const TArray<APBBallBase*> PreviousBalls = CaptureDeploymentSlotBalls();

	DeploymentSlots[SlotIndex].Ball = nullptr;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBalls);
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

	const TArray<APBBallBase*> PreviousBalls = CaptureDeploymentSlotBalls();

	Swap(DeploymentSlots[FirstIndex].Ball, DeploymentSlots[SecondIndex].Ball);
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBalls);
	return true;
}

bool UPBBallDeckSubsystem::IsDeploymentSlotValid(int32 SlotIndex) const
{
	return DeploymentSlots.IsValidIndex(SlotIndex);
}

bool UPBBallDeckSubsystem::IsDeploymentSlotOccupied(int32 SlotIndex) const
{
	return IsDeploymentSlotValid(SlotIndex) && DeploymentSlots[SlotIndex].Ball != nullptr;
}

APBBallBase* UPBBallDeckSubsystem::GetDeploymentSlotBall(int32 SlotIndex) const
{
	return IsDeploymentSlotValid(SlotIndex) ? DeploymentSlots[SlotIndex].Ball : nullptr;
}

APBBallBase* UPBBallDeckSubsystem::GetLeaderBall() const
{
	return GetDeploymentSlotBall(0);
}

TArray<APBBallBase*> UPBBallDeckSubsystem::GetDeploymentBalls() const
{
	TArray<APBBallBase*> Balls;
	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.Ball)
		{
			Balls.Add(DeploymentSlot.Ball);	
		}
	}
	return Balls;
}

TArray<APBBallBase*> UPBBallDeckSubsystem::GetFollowerBalls() const
{
	TArray<APBBallBase*> Balls;
	for (int32 SlotIndex = 1; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		if (DeploymentSlots[SlotIndex].Ball)
		{
			Balls.Add(DeploymentSlots[SlotIndex].Ball);
		}
	}
	return Balls;
}

int32 UPBBallDeckSubsystem::GetDeploymentBallCount() const
{
	int32 BallCount = 0;
	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.Ball)
		{
			++BallCount;
		}
	}
	return BallCount;
}

bool UPBBallDeckSubsystem::HasLeaderBall() const
{
	return GetLeaderBall() != nullptr;
}

bool UPBBallDeckSubsystem::CanBuildDeploymentParty() const
{
	return HasLeaderBall() && GetDeploymentBallCount() > 0;
}

void UPBBallDeckSubsystem::CompactDeploymentSlots()
{
	const TArray<APBBallBase*> PreviousBalls = CaptureDeploymentSlotBalls();
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBalls);
}

bool UPBBallDeckSubsystem::RotateDeploymentSlots()
{
	if (GetDeploymentBallCount() < 2)
	{
		return false;
	}

	CompactDeploymentSlotsInternal();

	APBBallBase* PreviousLeaderBall = DeploymentSlots[0].Ball;
	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num() - 1; ++SlotIndex)
	{
		DeploymentSlots[SlotIndex].Ball = DeploymentSlots[SlotIndex + 1].Ball;
	}
	DeploymentSlots.Last().Ball = PreviousLeaderBall;

	CompactDeploymentSlotsInternal();
	OnDeploymentSlotsRotated.Broadcast();
	return true;
}

TArray<APBBallBase*> UPBBallDeckSubsystem::CaptureDeploymentSlotBalls() const
{
	TArray<APBBallBase*> Balls;
	Balls.Reserve(DeploymentSlots.Num());
	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		Balls.Add(DeploymentSlot.Ball);
	}
	return Balls;
}

bool UPBBallDeckSubsystem::CompactDeploymentSlotsInternal()
{
	bool bChanged = false;
	TArray<TObjectPtr<APBBallBase>> OrderedBalls;
	OrderedBalls.Reserve(MaxDeploymentSlotCount);

	for (const FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.Ball)
		{
			OrderedBalls.Add(DeploymentSlot.Ball);
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		APBBallBase* PreviousBall = DeploymentSlots[SlotIndex].Ball;
		DeploymentSlots[SlotIndex].Ball = OrderedBalls.IsValidIndex(SlotIndex)
			? OrderedBalls[SlotIndex]
			: nullptr;

		if (PreviousBall != DeploymentSlots[SlotIndex].Ball)
		{
			bChanged = true;
		}
	}

	return bChanged;
}

void UPBBallDeckSubsystem::BroadcastDeploymentSlotChange(const TArray<APBBallBase*>& PreviousBalls)
{
	int32 ChangedSlotIndex = INDEX_NONE;
	APBBallBase* ChangedSlotBall = nullptr;
	int32 ChangedSlotCount = 0;

	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		APBBallBase* PreviousBall = PreviousBalls.IsValidIndex(SlotIndex) ? PreviousBalls[SlotIndex] : nullptr;
		APBBallBase* CurrentBall = DeploymentSlots[SlotIndex].Ball;
		if (PreviousBall == CurrentBall)
		{
			continue;
		}

		ChangedSlotIndex = SlotIndex;
		ChangedSlotBall = CurrentBall;
		++ChangedSlotCount;
	}

	if (ChangedSlotCount == 1)
	{
		OnDeploymentSlotChanged.Broadcast(ChangedSlotIndex, ChangedSlotBall);
	}
	else if (ChangedSlotCount > 1)
	{
		OnDeploymentSlotsReordered.Broadcast();
	}
}

void UPBBallDeckSubsystem::InitializeDeploymentSlots()
{
	DeploymentSlots.SetNum(MaxDeploymentSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num(); ++SlotIndex)
	{
		DeploymentSlots[SlotIndex].SlotIndex = SlotIndex;
		DeploymentSlots[SlotIndex].SlotType = EPBBallDeckSlotType::Deployment;
		DeploymentSlots[SlotIndex].Ball = nullptr;
	}
}

