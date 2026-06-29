// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckSubsystem.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"

void UPBBallDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDeploymentSlots();
	InitializeBenchSlots();
}

bool UPBBallDeckSubsystem::TestSnakeBall(TSubclassOf<APBBallBase> BallClass)
{
	if (!BallClass)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	TArray<APBBallBase*> SpawnedBalls;
	SpawnedBalls.Reserve(MaxDeploymentSlotCount);

	const FVector SpawnOrigin = FVector::ZeroVector;
	const float SpawnSpacing = 150.0f;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 BallIndex = 0; BallIndex < MaxDeploymentSlotCount; ++BallIndex)
	{
		const FVector SpawnLocation = SpawnOrigin + FVector(SpawnSpacing * BallIndex, 0.0f, 0.0f);
		APBBallBase* SpawnedBall = World->SpawnActor<APBBallBase>(BallClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
		if (!SpawnedBall)
		{
			for (APBBallBase* ExistingBall : SpawnedBalls)
			{
				if (ExistingBall)
				{
					ExistingBall->Destroy();
				}
			}
			return false;
		}

		SpawnedBalls.Add(SpawnedBall);
	}

	for (int32 SlotIndex = 0; SlotIndex < SpawnedBalls.Num(); ++SlotIndex)
	{
		if (!SetDeploymentSlot(SlotIndex, SpawnedBalls[SlotIndex]))
		{
			for (int32 CleanupSlotIndex = SlotIndex - 1; CleanupSlotIndex >= 0; --CleanupSlotIndex)
			{
				ClearDeploymentSlot(CleanupSlotIndex);
			}

			for (APBBallBase* ExistingBall : SpawnedBalls)
			{
				if (ExistingBall)
				{
					ExistingBall->Destroy();
				}
			}
			return false;
		}
	}

	return true;
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

#pragma endregion

#pragma region Bench

void UPBBallDeckSubsystem::InitializeBenchSlots()
{
	BenchSlots.SetNum(MaxBenchSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < BenchSlots.Num(); ++SlotIndex)
	{
		BenchSlots[SlotIndex].SlotIndex = SlotIndex;
		BenchSlots[SlotIndex].SlotType = EPBBallDeckSlotType::Bench;
		BenchSlots[SlotIndex].Ball = nullptr;
	}
}

bool UPBBallDeckSubsystem::SetBenchSlot(int32 SlotIndex, APBBallBase* Ball)
{
	if (!IsBenchSlotValid(SlotIndex) || Ball == nullptr)
	{
		return false;
	}

	BenchSlots[SlotIndex].Ball = Ball;
	OnBenchSlotChanged.Broadcast(SlotIndex, Ball);
	return true;
}

bool UPBBallDeckSubsystem::ClearBenchSlot(int32 SlotIndex)
{
	if (!IsBenchSlotValid(SlotIndex))
	{
		return false;
	}
	BenchSlots[SlotIndex].Ball = nullptr;
	OnBenchSlotChanged.Broadcast(SlotIndex, nullptr);
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

	Swap(BenchSlots[FirstIndex].Ball, BenchSlots[SecondIndex].Ball);
	OnBenchSlotsSwapped.Broadcast();
	return true;
}

bool UPBBallDeckSubsystem::IsBenchSlotValid(int32 SlotIndex) const
{
	return BenchSlots.IsValidIndex(SlotIndex);
}

bool UPBBallDeckSubsystem::IsBenchSlotOccupied(int32 SlotIndex) const
{
	return IsBenchSlotValid(SlotIndex) && BenchSlots[SlotIndex].Ball != nullptr;
}

bool UPBBallDeckSubsystem::HasEmptyBenchSlot() const
{
	return FindEmptyBenchSlot() != INDEX_NONE;
}

int32 UPBBallDeckSubsystem::FindEmptyBenchSlot() const
{
	for (int32 SlotIndex = 0; SlotIndex < BenchSlots.Num(); ++SlotIndex)
	{
		if (BenchSlots[SlotIndex].Ball == nullptr)
		{
			return SlotIndex;
		}
	}
	return INDEX_NONE;
}

bool UPBBallDeckSubsystem::AddBenchBall(APBBallBase* Ball)
{
	if (Ball == nullptr)
	{
		return false;
	}

	const int32 EmptySlotIndex = FindEmptyBenchSlot();
	if (EmptySlotIndex == INDEX_NONE)
	{
		return false;
	}

	return SetBenchSlot(EmptySlotIndex, Ball);
}

APBBallBase* UPBBallDeckSubsystem::GetBenchBall(int32 SlotIndex) const
{
	return IsBenchSlotValid(SlotIndex) ? BenchSlots[SlotIndex].Ball : nullptr;
}

TArray<APBBallBase*> UPBBallDeckSubsystem::GetBenchBalls() const
{
	TArray<APBBallBase*> Balls;
	for (const FPBBallDeckSlot& BenchSlot : BenchSlots)
	{
		if (BenchSlot.Ball)
		{
			Balls.Add(BenchSlot.Ball);
		}
	}
	return Balls;
}

int32 UPBBallDeckSubsystem::GetBenchBallCount() const
{
	int32 BallCount = 0;
	for (const FPBBallDeckSlot& BenchSlot : BenchSlots)
	{
		if (BenchSlot.Ball)
		{
			++BallCount;
		}
	}
	return BallCount;
}


#pragma endregion
