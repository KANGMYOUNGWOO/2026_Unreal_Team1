// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckSubsystem.h"

#include "PinBallLike/DataAsset/Ball/BPBallDataAsset.h"
#include "PinBallLike/Subsystem/BallDataSubsystem.h"

void UPBBallDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDeckSlots();
}

int32 UPBBallDeckSubsystem::AddOwnedBall(int32 BallId, int32 StarLevel)
{
	if (BallId == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem AddOwnedBall failed. Invalid BallId=%d"), BallId);
		return INDEX_NONE;
	}

	const int32 NewInstanceId = NextBallInstanceId++;
	FPBBallInstanceData& NewBallData = OwnedBallDataMap.Add(NewInstanceId);
	NewBallData.InstanceId = NewInstanceId;
	NewBallData.BallId = BallId;
	NewBallData.StarLevel = FMath::Max(StarLevel, 1);

	UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem AddOwnedBall succeeded. BallInstanceId=%d BallId=%d StarLevel=%d OwnedCount=%d"),
		NewBallData.InstanceId,
		NewBallData.BallId,
		NewBallData.StarLevel,
		OwnedBallDataMap.Num());
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

bool UPBBallDeckSubsystem::FindBallLocation(int32 BallInstanceId, FPBBallDeckSlot& OutLocation) const
{
	OutLocation = FPBBallDeckSlot();

	if (BallInstanceId == INDEX_NONE)
	{
		return false;
	}

	for (const FPBBallDeckSlot& DeckSlot : DeckSlots)
	{
		if (DeckSlot.BallInstanceId == BallInstanceId)
		{
			OutLocation = DeckSlot;
			return true;
		}
	}

	return false;
}

bool UPBBallDeckSubsystem::IsSlotValid(EPBBallDeckSlotType SlotType, int32 SlotIndex) const
{
	return GetDeckSlot(SlotType, SlotIndex) != nullptr;
}

bool UPBBallDeckSubsystem::IsSlotOccupied(EPBBallDeckSlotType SlotType, int32 SlotIndex) const
{
	const FPBBallDeckSlot* DeckSlot = GetDeckSlot(SlotType, SlotIndex);
	return DeckSlot && DeckSlot->BallInstanceId != INDEX_NONE;
}

int32 UPBBallDeckSubsystem::FindEmptySlot(EPBBallDeckSlotType SlotType) const
{
	const int32 SlotCount = SlotType == EPBBallDeckSlotType::Deployment
		? MaxDeploymentSlotCount
		: MaxBenchSlotCount;

	for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
	{
		if (!IsSlotOccupied(SlotType, SlotIndex))
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

int32 UPBBallDeckSubsystem::GetSlotBallInstanceId(EPBBallDeckSlotType SlotType, int32 SlotIndex) const
{
	const FPBBallDeckSlot* DeckSlot = GetDeckSlot(SlotType, SlotIndex);
	return DeckSlot ? DeckSlot->BallInstanceId : INDEX_NONE;
}

TArray<int32> UPBBallDeckSubsystem::GetSlotBallInstanceIds(EPBBallDeckSlotType SlotType) const
{
	TArray<int32> BallInstanceIds;
	for (const FPBBallDeckSlot& DeckSlot : DeckSlots)
	{
		if (DeckSlot.SlotType == SlotType && DeckSlot.BallInstanceId != INDEX_NONE)
		{
			BallInstanceIds.Add(DeckSlot.BallInstanceId);
		}
	}
	return BallInstanceIds;
}

TArray<int32> UPBBallDeckSubsystem::GetAllPlacedBallInstanceIds() const
{
	TArray<int32> BallInstanceIds = GetSlotBallInstanceIds(EPBBallDeckSlotType::Deployment);
	BallInstanceIds.Append(GetSlotBallInstanceIds(EPBBallDeckSlotType::Bench));
	return BallInstanceIds;
}

bool UPBBallDeckSubsystem::AddNewBallToDeck(int32 BallId, int32 StarLevel)
{
	const int32 EmptyDeploymentSlotIndex = FindEmptySlot(EPBBallDeckSlotType::Deployment);
	const int32 EmptyBenchSlotIndex = EmptyDeploymentSlotIndex == INDEX_NONE ? FindEmptySlot(EPBBallDeckSlotType::Bench) : INDEX_NONE;
	if (EmptyDeploymentSlotIndex == INDEX_NONE && EmptyBenchSlotIndex == INDEX_NONE)
	{
		return false;
	}

	const int32 NewBallInstanceId = AddOwnedBall(BallId, StarLevel);
	if (NewBallInstanceId == INDEX_NONE)
	{
		return false;
	}

	if (EmptyDeploymentSlotIndex != INDEX_NONE)
	{
		return SetDeploymentSlot(EmptyDeploymentSlotIndex, NewBallInstanceId);
	}

	return SetBenchSlot(EmptyBenchSlotIndex, NewBallInstanceId);
}

bool UPBBallDeckSubsystem::RemoveOwnedBall(int32 BallInstanceId)
{
	if (!HasOwnedBall(BallInstanceId))
	{
		return false;
	}

	FPBBallDeckSlot BallLocation;
	const bool bWasPlaced = FindBallLocation(BallInstanceId, BallLocation);
	if (bWasPlaced)
	{
		if (BallLocation.SlotType == EPBBallDeckSlotType::Deployment)
		{
			const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();
			if (FPBBallDeckSlot* DeckSlot = GetMutableDeckSlot(BallLocation.SlotType, BallLocation.SlotIndex))
			{
				DeckSlot->BallInstanceId = INDEX_NONE;
				CompactDeploymentSlotsInternal();
				BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
			}
		}
		else if (FPBBallDeckSlot* DeckSlot = GetMutableDeckSlot(BallLocation.SlotType, BallLocation.SlotIndex))
		{
			DeckSlot->BallInstanceId = INDEX_NONE;
			OnBenchSlotChanged.Broadcast(BallLocation.SlotIndex, INDEX_NONE);
		}
	}

	OwnedBallDataMap.Remove(BallInstanceId);
	return true;
}

bool UPBBallDeckSubsystem::BuildBallItemViewData(int32 BallInstanceId, EPBBallDeckSlotType SourceSlotType, int32 SourceSlotIndex, FPBBallItemViewData& OutViewData) const
{
	OutViewData = FPBBallItemViewData();

	const FPBBallInstanceData* BallInstanceData = GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData failed. Invalid instance. BallInstanceId=%d HasInstance=%s"),
			BallInstanceId,
			BallInstanceData ? TEXT("true") : TEXT("false"));
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UBallDataSubsystem* BallDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UBallDataSubsystem>() : nullptr;
	const UPBBallDataAsset* BallDataAsset = BallDataSubsystem ? BallDataSubsystem->GetBallDataAsset(BallInstanceData->BallId) : nullptr;

	if (!BallDataSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData failed. BallDataSubsystem is null. BallInstanceId=%d BallId=%d"),
			BallInstanceData->InstanceId,
			BallInstanceData->BallId);
		return false;
	}

	if (!BallDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData missing DataAsset. BallInstanceId=%d BallId=%d SlotType=%d SlotIndex=%d"),
			BallInstanceData->InstanceId,
			BallInstanceData->BallId,
			static_cast<int32>(SourceSlotType),
			SourceSlotIndex);
	}
	else if (!BallDataAsset->Icon)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData DataAsset has no Icon. BallInstanceId=%d BallId=%d Asset=%s"),
			BallInstanceData->InstanceId,
			BallInstanceData->BallId,
			*GetNameSafe(BallDataAsset));
	}

	OutViewData.BallInstanceId = BallInstanceData->InstanceId;
	OutViewData.BallId = BallInstanceData->BallId;
	OutViewData.StarLevel = BallInstanceData->StarLevel;
	OutViewData.SourceSlotType = SourceSlotType;
	OutViewData.SourceSlotIndex = SourceSlotIndex;
	OutViewData.Icon = BallDataAsset ? BallDataAsset->Icon : nullptr;

	UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData finished. BallInstanceId=%d BallId=%d StarLevel=%d SlotType=%d SlotIndex=%d DataAsset=%s Icon=%s"),
		OutViewData.BallInstanceId,
		OutViewData.BallId,
		OutViewData.StarLevel,
		static_cast<int32>(OutViewData.SourceSlotType),
		OutViewData.SourceSlotIndex,
		*GetNameSafe(BallDataAsset),
		*GetNameSafe(OutViewData.Icon));
	return true;
}

bool UPBBallDeckSubsystem::MoveBallBetweenSlots(EPBBallDeckSlotType SourceSlotType, int32 SourceSlotIndex, EPBBallDeckSlotType TargetSlotType, int32 TargetSlotIndex)
{
	FPBBallDeckSlot* SourceSlot = GetMutableDeckSlot(SourceSlotType, SourceSlotIndex);
	FPBBallDeckSlot* TargetSlot = GetMutableDeckSlot(TargetSlotType, TargetSlotIndex);
	if (!SourceSlot || !TargetSlot || SourceSlot->BallInstanceId == INDEX_NONE)
	{
		return false;
	}

	if (SourceSlotType == TargetSlotType && SourceSlotIndex == TargetSlotIndex)
	{
		return true;
	}

	const int32 SourceBallInstanceId = SourceSlot->BallInstanceId;
	const int32 TargetBallInstanceId = TargetSlot->BallInstanceId;
	const bool bSourceIsDeployment = SourceSlotType == EPBBallDeckSlotType::Deployment;
	const bool bTargetIsDeployment = TargetSlotType == EPBBallDeckSlotType::Deployment;
	const bool bTouchesDeployment = bSourceIsDeployment || bTargetIsDeployment;
	const TArray<int32> PreviousDeploymentBallInstanceIds = bTouchesDeployment
		? CaptureDeploymentSlotBallInstanceIds()
		: TArray<int32>();

	SourceSlot->BallInstanceId = TargetBallInstanceId;
	TargetSlot->BallInstanceId = SourceBallInstanceId;

	if (bTouchesDeployment)
	{
		CompactDeploymentSlotsInternal();
		BroadcastDeploymentSlotChange(PreviousDeploymentBallInstanceIds);
	}

	if (SourceSlotType == EPBBallDeckSlotType::Bench)
	{
		OnBenchSlotChanged.Broadcast(SourceSlotIndex, TargetBallInstanceId);
	}

	if (TargetSlotType == EPBBallDeckSlotType::Bench)
	{
		OnBenchSlotChanged.Broadcast(TargetSlotIndex, SourceBallInstanceId);
	}

	return true;
}

#pragma region Deployment Slot

EPBBallPartyRole UPBBallDeckSubsystem::GetDeploymentRole(int32 SlotIndex) const
{
	if (!IsSlotValid(EPBBallDeckSlotType::Deployment, SlotIndex))
	{
		return EPBBallPartyRole::None;
	}

	return SlotIndex == 0
		? EPBBallPartyRole::Leader
		: EPBBallPartyRole::Follower;
}

bool UPBBallDeckSubsystem::SetDeploymentSlot(int32 SlotIndex, int32 BallInstanceId)
{
	if (!IsSlotValid(EPBBallDeckSlotType::Deployment, SlotIndex) || !HasOwnedBall(BallInstanceId))
	{
		return false;
	}

	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();

	ClearBallInstanceFromSlots(BallInstanceId);
	GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, SlotIndex)->BallInstanceId = BallInstanceId;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	return true;
}

bool UPBBallDeckSubsystem::ClearDeploymentSlot(int32 SlotIndex)
{
	if (!IsSlotValid(EPBBallDeckSlotType::Deployment, SlotIndex))
	{
		return false;
	}

	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();

	GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, SlotIndex)->BallInstanceId = INDEX_NONE;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	return true;
}

bool UPBBallDeckSubsystem::SwapDeploymentSlots(int32 FirstIndex, int32 SecondIndex)
{
	if (!IsSlotValid(EPBBallDeckSlotType::Deployment, FirstIndex) || !IsSlotValid(EPBBallDeckSlotType::Deployment, SecondIndex))
	{
		return false;
	}

	if (FirstIndex == SecondIndex)
	{
		return true;
	}

	const TArray<int32> PreviousBallInstanceIds = CaptureDeploymentSlotBallInstanceIds();

	FPBBallDeckSlot* FirstSlot = GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, FirstIndex);
	FPBBallDeckSlot* SecondSlot = GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, SecondIndex);
	Swap(FirstSlot->BallInstanceId, SecondSlot->BallInstanceId);
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	return true;
}

int32 UPBBallDeckSubsystem::GetLeaderBallInstanceId() const
{
	return GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, 0);
}

TArray<int32> UPBBallDeckSubsystem::GetDeploymentBallInstanceIds() const
{
	return GetSlotBallInstanceIds(EPBBallDeckSlotType::Deployment);
}

TArray<int32> UPBBallDeckSubsystem::GetFollowerBallInstanceIds() const
{
	TArray<int32> BallInstanceIds;
	for (int32 SlotIndex = 1; SlotIndex < MaxDeploymentSlotCount; ++SlotIndex)
	{
		const int32 BallInstanceId = GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, SlotIndex);
		if (BallInstanceId != INDEX_NONE)
		{
			BallInstanceIds.Add(BallInstanceId);
		}
	}
	return BallInstanceIds;
}

int32 UPBBallDeckSubsystem::GetDeploymentBallCount() const
{
	return GetDeploymentBallInstanceIds().Num();
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

	const int32 PreviousLeaderBallInstanceId = GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, 0);
	for (int32 SlotIndex = 0; SlotIndex < MaxDeploymentSlotCount - 1; ++SlotIndex)
	{
		FPBBallDeckSlot* CurrentSlot = GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, SlotIndex);
		const FPBBallDeckSlot* NextSlot = GetDeckSlot(EPBBallDeckSlotType::Deployment, SlotIndex + 1);
		CurrentSlot->BallInstanceId = NextSlot ? NextSlot->BallInstanceId : INDEX_NONE;
	}
	GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, MaxDeploymentSlotCount - 1)->BallInstanceId = PreviousLeaderBallInstanceId;

	CompactDeploymentSlotsInternal();
	OnDeploymentSlotsRotated.Broadcast();
	return true;
}

TArray<int32> UPBBallDeckSubsystem::CaptureDeploymentSlotBallInstanceIds() const
{
	TArray<int32> BallInstanceIds;
	BallInstanceIds.Reserve(MaxDeploymentSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < MaxDeploymentSlotCount; ++SlotIndex)
	{
		BallInstanceIds.Add(GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, SlotIndex));
	}
	return BallInstanceIds;
}

bool UPBBallDeckSubsystem::CompactDeploymentSlotsInternal()
{
	bool bChanged = false;
	TArray<int32> OrderedBallInstanceIds;
	OrderedBallInstanceIds.Reserve(MaxDeploymentSlotCount);

	for (int32 SlotIndex = 0; SlotIndex < MaxDeploymentSlotCount; ++SlotIndex)
	{
		const int32 BallInstanceId = GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, SlotIndex);
		if (BallInstanceId != INDEX_NONE)
		{
			OrderedBallInstanceIds.Add(BallInstanceId);
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < MaxDeploymentSlotCount; ++SlotIndex)
	{
		FPBBallDeckSlot* DeploymentSlot = GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, SlotIndex);
		const int32 PreviousBallInstanceId = DeploymentSlot->BallInstanceId;
		DeploymentSlot->BallInstanceId = OrderedBallInstanceIds.IsValidIndex(SlotIndex)
			? OrderedBallInstanceIds[SlotIndex]
			: INDEX_NONE;

		if (PreviousBallInstanceId != DeploymentSlot->BallInstanceId)
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

	for (int32 SlotIndex = 0; SlotIndex < MaxDeploymentSlotCount; ++SlotIndex)
	{
		const int32 PreviousBallInstanceId = PreviousBallInstanceIds.IsValidIndex(SlotIndex)
			? PreviousBallInstanceIds[SlotIndex]
			: INDEX_NONE;
		const int32 CurrentBallInstanceId = GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, SlotIndex);
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

	for (FPBBallDeckSlot& DeckSlot : DeckSlots)
	{
		if (DeckSlot.BallInstanceId == BallInstanceId)
		{
			DeckSlot.BallInstanceId = INDEX_NONE;
		}
	}
}

void UPBBallDeckSubsystem::InitializeDeckSlots()
{
	DeckSlots.SetNum(MaxDeckSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < MaxDeploymentSlotCount; ++SlotIndex)
	{
		FPBBallDeckSlot& DeckSlot = DeckSlots[DeploymentSlotStartIndex + SlotIndex];
		DeckSlot.SlotIndex = SlotIndex;
		DeckSlot.SlotType = EPBBallDeckSlotType::Deployment;
		DeckSlot.BallInstanceId = INDEX_NONE;
	}

	for (int32 SlotIndex = 0; SlotIndex < MaxBenchSlotCount; ++SlotIndex)
	{
		FPBBallDeckSlot& DeckSlot = DeckSlots[BenchSlotStartIndex + SlotIndex];
		DeckSlot.SlotIndex = SlotIndex;
		DeckSlot.SlotType = EPBBallDeckSlotType::Bench;
		DeckSlot.BallInstanceId = INDEX_NONE;
	}
}

#pragma endregion

#pragma region Bench

bool UPBBallDeckSubsystem::SetBenchSlot(int32 SlotIndex, int32 BallInstanceId)
{
	if (!IsSlotValid(EPBBallDeckSlotType::Bench, SlotIndex) || !HasOwnedBall(BallInstanceId))
	{
		return false;
	}

	ClearBallInstanceFromSlots(BallInstanceId);
	GetMutableDeckSlot(EPBBallDeckSlotType::Bench, SlotIndex)->BallInstanceId = BallInstanceId;
	OnBenchSlotChanged.Broadcast(SlotIndex, BallInstanceId);
	return true;
}

bool UPBBallDeckSubsystem::ClearBenchSlot(int32 SlotIndex)
{
	if (!IsSlotValid(EPBBallDeckSlotType::Bench, SlotIndex))
	{
		return false;
	}
	GetMutableDeckSlot(EPBBallDeckSlotType::Bench, SlotIndex)->BallInstanceId = INDEX_NONE;
	OnBenchSlotChanged.Broadcast(SlotIndex, INDEX_NONE);
	return true;
}

bool UPBBallDeckSubsystem::SwapBenchSlots(int32 FirstIndex, int32 SecondIndex)
{
	if (!IsSlotValid(EPBBallDeckSlotType::Bench, FirstIndex) || !IsSlotValid(EPBBallDeckSlotType::Bench, SecondIndex))
	{
		return false;
	}

	if (FirstIndex == SecondIndex)
	{
		return true;
	}

	FPBBallDeckSlot* FirstSlot = GetMutableDeckSlot(EPBBallDeckSlotType::Bench, FirstIndex);
	FPBBallDeckSlot* SecondSlot = GetMutableDeckSlot(EPBBallDeckSlotType::Bench, SecondIndex);
	Swap(FirstSlot->BallInstanceId, SecondSlot->BallInstanceId);
	OnBenchSlotsSwapped.Broadcast();
	return true;
}

TArray<int32> UPBBallDeckSubsystem::GetBenchBallInstanceIds() const
{
	return GetSlotBallInstanceIds(EPBBallDeckSlotType::Bench);
}

int32 UPBBallDeckSubsystem::GetBenchBallCount() const
{
	return GetBenchBallInstanceIds().Num();
}


#pragma endregion

int32 UPBBallDeckSubsystem::ToGlobalSlotIndex(EPBBallDeckSlotType SlotType, int32 SlotIndex) const
{
	switch (SlotType)
	{
	case EPBBallDeckSlotType::Deployment:
		return SlotIndex >= 0 && SlotIndex < MaxDeploymentSlotCount
			? DeploymentSlotStartIndex + SlotIndex
			: INDEX_NONE;
	case EPBBallDeckSlotType::Bench:
	default:
		return SlotIndex >= 0 && SlotIndex < MaxBenchSlotCount
			? BenchSlotStartIndex + SlotIndex
			: INDEX_NONE;
	}
}

const FPBBallDeckSlot* UPBBallDeckSubsystem::GetDeckSlot(EPBBallDeckSlotType SlotType, int32 SlotIndex) const
{
	const int32 GlobalSlotIndex = ToGlobalSlotIndex(SlotType, SlotIndex);
	return DeckSlots.IsValidIndex(GlobalSlotIndex) ? &DeckSlots[GlobalSlotIndex] : nullptr;
}

FPBBallDeckSlot* UPBBallDeckSubsystem::GetMutableDeckSlot(EPBBallDeckSlotType SlotType, int32 SlotIndex)
{
	const int32 GlobalSlotIndex = ToGlobalSlotIndex(SlotType, SlotIndex);
	return DeckSlots.IsValidIndex(GlobalSlotIndex) ? &DeckSlots[GlobalSlotIndex] : nullptr;
}
