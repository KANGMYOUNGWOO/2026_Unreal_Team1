// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckSubsystem.h"

void UPBBallDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDeploymentSlots();
}

EPBDeploymentSlotRole UPBBallDeckSubsystem::GetDeploymentRole(int32 SlotIndex) const
{
	if (!IsDeploymentSlotValid(SlotIndex))
	{
		return EPBDeploymentSlotRole::None;
	}

	return SlotIndex == 0
		? EPBDeploymentSlotRole::Leader
		: EPBDeploymentSlotRole::Follower;
}

bool UPBBallDeckSubsystem::SetDeploymentSlot(int32 SlotIndex, APBBallBase* Ball)
{
	if (!IsDeploymentSlotValid(SlotIndex) || Ball == nullptr)
	{
		return false;
	}

	for (FPBBallDeckSlot& DeploymentSlot : DeploymentSlots)
	{
		if (DeploymentSlot.Ball == Ball)
		{
			DeploymentSlot.Ball = nullptr;
		}
	}

	DeploymentSlots[SlotIndex].Ball = Ball;
	CompactDeploymentSlots();
	return true;
}

bool UPBBallDeckSubsystem::ClearDeploymentSlot(int32 SlotIndex)
{
	if (!IsDeploymentSlotValid(SlotIndex))
	{
		return false;
	}

	DeploymentSlots[SlotIndex].Ball = nullptr;
	CompactDeploymentSlots();
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

	Swap(DeploymentSlots[FirstIndex].Ball, DeploymentSlots[SecondIndex].Ball);
	CompactDeploymentSlots();
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
		DeploymentSlots[SlotIndex].Ball = OrderedBalls.IsValidIndex(SlotIndex)
			? OrderedBalls[SlotIndex]
			: nullptr;
	}
}

bool UPBBallDeckSubsystem::RotateDeploymentSlots()
{
	if (GetDeploymentBallCount() < 2)
	{
		return false;
	}

	CompactDeploymentSlots();

	APBBallBase* PreviousLeaderBall = DeploymentSlots[0].Ball;
	for (int32 SlotIndex = 0; SlotIndex < DeploymentSlots.Num() - 1; ++SlotIndex)
	{
		DeploymentSlots[SlotIndex].Ball = DeploymentSlots[SlotIndex + 1].Ball;
	}
	DeploymentSlots.Last().Ball = PreviousLeaderBall;

	CompactDeploymentSlots();
	return true;
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

