// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckSubsystem.h"

#include "PBBallDeckAssetLoadService.h"
#include "PBBallDeckFusionService.h"
#include "PBBallDeckSynergyService.h"

#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"

void UPBBallDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDeckSlots();

	FusionService = NewObject<UPBBallDeckFusionService>(this);
	if (FusionService)
	{
		FusionService->Initialize(this);
		FusionService->OnBallFusionStarted.AddDynamic(this, &UPBBallDeckSubsystem::HandleBallFusionStarted);
		FusionService->OnBallFusionCompleted.AddDynamic(this, &UPBBallDeckSubsystem::HandleBallFusionCompleted);
		FusionService->OnBallFusionCanceled.AddDynamic(this, &UPBBallDeckSubsystem::HandleBallFusionCanceled);
	}

	AssetLoadService = NewObject<UPBBallDeckAssetLoadService>(this);
	if (AssetLoadService)
	{
		AssetLoadService->Initialize(this);
	}

	SynergyService = NewObject<UPBBallDeckSynergyService>(this);
	if (SynergyService)
	{
		SynergyService->Initialize(this);
	}
}

void UPBBallDeckSubsystem::Deinitialize()
{
	if (SynergyService)
	{
		SynergyService->Deinitialize();
	}

	Super::Deinitialize();
}

int32 UPBBallDeckSubsystem::AddOwnedBall(FName BallId, int32 StarLevel)
{
	if (BallId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem AddOwnedBall failed. Invalid BallId=%s"), *BallId.ToString());
		return INDEX_NONE;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	if (!TableDataSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem AddOwnedBall failed. Missing TableDataSubsystem. BallId=%s"),
			*BallId.ToString());
		return INDEX_NONE;
	}

	FPBBallTableRow BallRow;
	if (!TableDataSubsystem->FindBallRow(BallId, BallRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem AddOwnedBall failed. Ball row not found. BallId=%s"),
			*BallId.ToString());
		return INDEX_NONE;
	}

	const bool bHadOwnedBallWithSameId = HasOwnedBallWithBallId(BallId);
	const int32 NewInstanceId = NextBallInstanceId++;
	FPBDeckOwnedBallData& NewBallData = OwnedBallDataMap.Add(NewInstanceId);
	NewBallData.InstanceId = NewInstanceId;
	NewBallData.BallId = BallId;
	NewBallData.StarLevel = FMath::Max(StarLevel, 1);

	UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem AddOwnedBall succeeded. BallInstanceId=%d BallId=%s StarLevel=%d OwnedCount=%d"),
		NewBallData.InstanceId,
		*NewBallData.BallId.ToString(),
		NewBallData.StarLevel,
		OwnedBallDataMap.Num());

	if (AssetLoadService && !bHadOwnedBallWithSameId)
	{
		AssetLoadService->LoadOwnedBallIconAsync(
			BallId,
			FStreamableDelegate::CreateUObject(
				this,
				&UPBBallDeckSubsystem::HandleOwnedBallIconLoaded,
				BallId));
	}

	return NewInstanceId;
}

const FPBDeckOwnedBallData* UPBBallDeckSubsystem::GetOwnedBallData(int32 BallInstanceId) const
{
	return OwnedBallDataMap.Find(BallInstanceId);
}

bool UPBBallDeckSubsystem::HasOwnedBall(int32 BallInstanceId) const
{
	return OwnedBallDataMap.Contains(BallInstanceId);
}

bool UPBBallDeckSubsystem::SetOwnedBallSavedMana(const int32 BallInstanceId, const float SavedMana)
{
	FPBDeckOwnedBallData* BallInstanceData = OwnedBallDataMap.Find(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		return false;
	}

	BallInstanceData->SavedMana = FMath::Max(SavedMana, 0.0f);
	return true;
}

float UPBBallDeckSubsystem::GetOwnedBallSavedMana(const int32 BallInstanceId) const
{
	const FPBDeckOwnedBallData* BallInstanceData = GetOwnedBallData(BallInstanceId);
	return BallInstanceData && BallInstanceData->IsValid() ? BallInstanceData->SavedMana : 0.0f;
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

bool UPBBallDeckSubsystem::AddNewBallToDeck(FName BallId, int32 StarLevel)
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
	const FPBDeckOwnedBallData* BallInstanceData = GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		return false;
	}

	const FName RemovedBallId = BallInstanceData->BallId;

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
	if (AssetLoadService && !HasOwnedBallWithBallId(RemovedBallId))
	{
		AssetLoadService->UnloadOwnedBallIcon(RemovedBallId);
	}

	return true;
}

bool UPBBallDeckSubsystem::SetOwnedBallStarLevel(int32 BallInstanceId, int32 StarLevel)
{
	FPBDeckOwnedBallData* BallInstanceData = OwnedBallDataMap.Find(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		return false;
	}

	BallInstanceData->StarLevel = FMath::Max(StarLevel, 1);

	FPBBallDeckSlot BallLocation;
	if (FindBallLocation(BallInstanceId, BallLocation))
	{
		if (BallLocation.SlotType == EPBBallDeckSlotType::Deployment)
		{
			OnDeploymentSlotChanged.Broadcast(BallLocation.SlotIndex, BallInstanceId);
		}
		else
		{
			OnBenchSlotChanged.Broadcast(BallLocation.SlotIndex, BallInstanceId);
		}
	}

	return true;
}

bool UPBBallDeckSubsystem::BuildBallItemViewData(int32 BallInstanceId, EPBBallDeckSlotType SourceSlotType, int32 SourceSlotIndex, FPBBallItemViewData& OutViewData) const
{
	OutViewData = FPBBallItemViewData();

	const FPBDeckOwnedBallData* BallInstanceData = GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData failed. Invalid instance. BallInstanceId=%d HasInstance=%s"),
			BallInstanceId,
			BallInstanceData ? TEXT("true") : TEXT("false"));
		return false;
	}

	OutViewData.BallInstanceId = BallInstanceData->InstanceId;
	OutViewData.BallId = BallInstanceData->BallId;
	OutViewData.StarLevel = BallInstanceData->StarLevel;
	OutViewData.SourceSlotType = SourceSlotType;
	OutViewData.SourceSlotIndex = SourceSlotIndex;

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;

	FPBBallTableRow BallRow;
	const bool bFoundBallRow = TableDataSubsystem
		&& !BallInstanceData->BallId.IsNone()
		&& TableDataSubsystem->FindBallRow(BallInstanceData->BallId, BallRow);
	const UPBBallDataAsset* BallDataAsset = nullptr;
	if (bFoundBallRow && AssetLoadService)
	{
		BallDataAsset = AssetLoadService->GetLoadedBallDataAsset(BallInstanceId);
		OutViewData.Icon = AssetLoadService->GetLoadedBallIcon(BallInstanceId);
	}
	else if (!bFoundBallRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData could not find ball row. BallInstanceId=%d BallId=%s TableDataSubsystem=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			TableDataSubsystem ? TEXT("valid") : TEXT("null"));
	}
	else if (!AssetLoadService)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData failed. AssetLoadService is null. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
	}

	UE_LOG(LogTemp, Warning, TEXT("BallDeckSubsystem BuildBallItemViewData finished. BallInstanceId=%d BallId=%s StarLevel=%d RowFound=%s SlotType=%d SlotIndex=%d DataAsset=%s Icon=%s"),
		OutViewData.BallInstanceId,
		*OutViewData.BallId.ToString(),
		OutViewData.StarLevel,
		bFoundBallRow ? TEXT("true") : TEXT("false"),
		static_cast<int32>(OutViewData.SourceSlotType),
		OutViewData.SourceSlotIndex,
		*GetNameSafe(BallDataAsset),
		*GetNameSafe(OutViewData.Icon));
	return true;
}

FGuid UPBBallDeckSubsystem::LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate OnLoaded)
{
	return AssetLoadService
		? AssetLoadService->LoadPlacedBallGameplayAssetsAsync(OnLoaded)
		: FGuid();
}

FGuid UPBBallDeckSubsystem::LoadPlacedBallUIAssetsAsync(FStreamableDelegate OnLoaded)
{
	return AssetLoadService
		? AssetLoadService->LoadPlacedBallUIAssetsAsync(OnLoaded)
		: FGuid();
}

void UPBBallDeckSubsystem::UnloadPlacedBallGameplayAssets()
{
	if (AssetLoadService)
	{
		AssetLoadService->UnloadGameplayAssets();
	}
}

void UPBBallDeckSubsystem::UnloadPlacedBallUIAssets()
{
	if (AssetLoadService)
	{
		AssetLoadService->UnloadUIAssets();
	}
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

	ResetSavedManaIfEnteringDeployment(SourceBallInstanceId, SourceSlotType, TargetSlotType);
	ResetSavedManaIfEnteringDeployment(TargetBallInstanceId, TargetSlotType, SourceSlotType);

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

int32 UPBBallDeckSubsystem::GetSellPrice(int32 BallInstanceId)
{
	const FPBDeckOwnedBallData* BallInstanceData = GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		return 0;
	}

	UGameInstance* GameInstance = GetGameInstance();
	
	return 0;
}

bool UPBBallDeckSubsystem::SellBall(int32 BallInstanceId, int32& OutSellPrice)
{
	OutSellPrice = GetSellPrice(BallInstanceId);
	if (OutSellPrice <= 0)
	{
		return false;
	}

	const FPBDeckOwnedBallData* BallInstanceData = GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		OutSellPrice = 0;
		return false;
	}

	const FName BallId = BallInstanceData->BallId;
	if (!RemoveOwnedBall(BallInstanceId))
	{
		OutSellPrice = 0;
		return false;
	}

	OnBallSold.Broadcast(BallInstanceId, BallId, OutSellPrice);
	return true;
}

#pragma region Fusion

UPBBallDeckFusionService* UPBBallDeckSubsystem::GetFusionService() const
{
	return FusionService;
}

UPBBallDeckAssetLoadService* UPBBallDeckSubsystem::GetAssetLoadService() const
{
	return AssetLoadService;
}

UPBBallDeckSynergyService* UPBBallDeckSubsystem::GetSynergyService() const
{
	return SynergyService;
}

bool UPBBallDeckSubsystem::TryStartFusion()
{
	UE_LOG(LogTemp, Warning, TEXT("TryStartFusion"));
	return FusionService && FusionService->TryStartFusion();
}

bool UPBBallDeckSubsystem::CompletePendingFusion()
{
	UE_LOG(LogTemp, Warning, TEXT("CompletePendingFusion"));
	return FusionService && FusionService->CompletePendingFusion();
}

bool UPBBallDeckSubsystem::CancelPendingFusion()
{
	UE_LOG(LogTemp, Warning, TEXT("CancelPendingFusion"));
	return FusionService && FusionService->CancelPendingFusion();
}

bool UPBBallDeckSubsystem::HasPendingFusion() const
{
	UE_LOG(LogTemp, Warning, TEXT("HasPendingFusion"));
	return FusionService && FusionService->HasPendingFusion();
}

void UPBBallDeckSubsystem::HandleBallFusionStarted(const FPBBallDeckFusionBatch& FusionBatch)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleBallFusionStarted"));
	OnBallFusionStarted.Broadcast(FusionBatch);
	CompletePendingFusion();
}

void UPBBallDeckSubsystem::HandleBallFusionCompleted(const FPBBallDeckFusionBatch& FusionBatch)
{
	for (const FPBBallDeckFusionRequest& FusionRequest : FusionBatch.FusionRequests)
	{
		SetOwnedBallSavedMana(FusionRequest.SurvivorBallInstanceId, 0.0f);
	}

	OnBallFusionCompleted.Broadcast(FusionBatch);
}

void UPBBallDeckSubsystem::HandleBallFusionCanceled(const FPBBallDeckFusionBatch& FusionBatch)
{
	OnBallFusionCanceled.Broadcast(FusionBatch);
}

#pragma endregion

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
	FPBBallDeckSlot PreviousLocation;
	const EPBBallDeckSlotType PreviousSlotType = FindBallLocation(BallInstanceId, PreviousLocation)
		? PreviousLocation.SlotType
		: EPBBallDeckSlotType::Bench;

	ClearBallInstanceFromSlots(BallInstanceId);
	ResetSavedManaIfEnteringDeployment(BallInstanceId, PreviousSlotType, EPBBallDeckSlotType::Deployment);
	GetMutableDeckSlot(EPBBallDeckSlotType::Deployment, SlotIndex)->BallInstanceId = BallInstanceId;
	CompactDeploymentSlotsInternal();
	BroadcastDeploymentSlotChange(PreviousBallInstanceIds);
	TryStartFusion();
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

void UPBBallDeckSubsystem::HandleOwnedBallIconLoaded(const FName BallId)
{
	if (BallId.IsNone())
	{
		return;
	}

	for (const FPBBallDeckSlot& DeckSlot : DeckSlots)
	{
		if (DeckSlot.BallInstanceId == INDEX_NONE)
		{
			continue;
		}

		const FPBDeckOwnedBallData* BallInstanceData = GetOwnedBallData(DeckSlot.BallInstanceId);
		if (!BallInstanceData || BallInstanceData->BallId != BallId)
		{
			continue;
		}

		if (DeckSlot.SlotType == EPBBallDeckSlotType::Deployment)
		{
			OnDeploymentSlotChanged.Broadcast(DeckSlot.SlotIndex, DeckSlot.BallInstanceId);
		}
		else
		{
			OnBenchSlotChanged.Broadcast(DeckSlot.SlotIndex, DeckSlot.BallInstanceId);
		}
	}
}

void UPBBallDeckSubsystem::ResetSavedManaIfEnteringDeployment(
	const int32 BallInstanceId,
	const EPBBallDeckSlotType SourceSlotType,
	const EPBBallDeckSlotType TargetSlotType)
{
	if (BallInstanceId == INDEX_NONE)
	{
		return;
	}

	if (SourceSlotType != EPBBallDeckSlotType::Deployment && TargetSlotType == EPBBallDeckSlotType::Deployment)
	{
		SetOwnedBallSavedMana(BallInstanceId, 0.0f);
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
	TryStartFusion();
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

bool UPBBallDeckSubsystem::HasOwnedBallWithBallId(const FName BallId) const
{
	if (BallId.IsNone())
	{
		return false;
	}

	for (const TPair<int32, FPBDeckOwnedBallData>& OwnedBallDataPair : OwnedBallDataMap)
	{
		if (OwnedBallDataPair.Value.BallId == BallId)
		{
			return true;
		}
	}

	return false;
}
