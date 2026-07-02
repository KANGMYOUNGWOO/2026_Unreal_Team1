// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckFusionService.h"

#include "PBBallDeckSubsystem.h"
#include "PinBallLike/Struct/Deck/PBBallInstanceData.h"

void UPBBallDeckFusionService::Initialize(UPBBallDeckSubsystem* InDeckSubsystem)
{
	DeckSubsystem = InDeckSubsystem;
	PendingFusion = FPBBallDeckFusionRequest();
}

bool UPBBallDeckFusionService::TryStartFusion()
{
	if (HasPendingFusion())
	{
		return false;
	}

	FPBBallDeckFusionRequest FusionRequest;
	if (!BuildNextFusionRequest(FusionRequest))
	{
		return false;
	}

	PendingFusion = FusionRequest;
	OnBallFusionStarted.Broadcast(PendingFusion);
	return true;
}

bool UPBBallDeckFusionService::CompletePendingFusion()
{
	if (!HasPendingFusion())
	{
		return false;
	}

	const FPBBallDeckFusionRequest CompletedFusion = PendingFusion;
	if (!ApplyFusionRequest(CompletedFusion))
	{
		return false;
	}

	PendingFusion = FPBBallDeckFusionRequest();
	OnBallFusionCompleted.Broadcast(CompletedFusion);
	return true;
}

bool UPBBallDeckFusionService::CancelPendingFusion()
{
	if (!HasPendingFusion())
	{
		return false;
	}

	const FPBBallDeckFusionRequest CanceledFusion = PendingFusion;
	PendingFusion = FPBBallDeckFusionRequest();
	OnBallFusionCanceled.Broadcast(CanceledFusion);
	return true;
}

bool UPBBallDeckFusionService::HasPendingFusion() const
{
	return PendingFusion.IsValid();
}

FPBBallDeckFusionRequest UPBBallDeckFusionService::GetPendingFusion() const
{
	return PendingFusion;
}

bool UPBBallDeckFusionService::BuildNextFusionRequest(FPBBallDeckFusionRequest& OutFusionRequest) const
{
	OutFusionRequest = FPBBallDeckFusionRequest();

	if (!DeckSubsystem)
	{
		return false;
	}

	TMap<int64, TArray<int32>> FusionCandidatesByKey;
	const TArray<int32> PlacedBallInstanceIds = DeckSubsystem->GetAllPlacedBallInstanceIds();
	for (const int32 BallInstanceId : PlacedBallInstanceIds)
	{
		const FPBBallInstanceData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
		if (!BallInstanceData || !BallInstanceData->IsValid())
		{
			continue;
		}

		const int64 CandidateKey = (static_cast<int64>(BallInstanceData->BallId) << 32)
			| static_cast<uint32>(BallInstanceData->StarLevel);
		TArray<int32>& CandidateBallInstanceIds = FusionCandidatesByKey.FindOrAdd(CandidateKey);
		CandidateBallInstanceIds.Add(BallInstanceId);

		if (CandidateBallInstanceIds.Num() < FPBBallDeckFusionRequest::RequiredFusionBallCount)
		{
			continue;
		}

		const int32 SurvivorBallInstanceId = CandidateBallInstanceIds[0];
		FPBBallDeckSlot SurvivorLocation;
		if (!DeckSubsystem->FindBallLocation(SurvivorBallInstanceId, SurvivorLocation))
		{
			return false;
		}

		OutFusionRequest.SurvivorBallInstanceId = SurvivorBallInstanceId;
		OutFusionRequest.ConsumedBallInstanceIds = {
			CandidateBallInstanceIds[0],
			CandidateBallInstanceIds[1],
			CandidateBallInstanceIds[2]
		};
		OutFusionRequest.SurvivorSlotType = SurvivorLocation.SlotType;
		OutFusionRequest.SurvivorSlotIndex = SurvivorLocation.SlotIndex;
		OutFusionRequest.BallId = BallInstanceData->BallId;
		OutFusionRequest.SourceStarLevel = BallInstanceData->StarLevel;
		OutFusionRequest.ResultStarLevel = BallInstanceData->StarLevel + 1;
		return true;
	}

	return false;
}

bool UPBBallDeckFusionService::ApplyFusionRequest(const FPBBallDeckFusionRequest& FusionRequest) const
{
	if (!DeckSubsystem || !FusionRequest.IsValid())
	{
		return false;
	}

	for (const int32 BallInstanceId : FusionRequest.ConsumedBallInstanceIds)
	{
		const FPBBallInstanceData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
		if (!BallInstanceData
			|| !BallInstanceData->IsValid()
			|| BallInstanceData->BallId != FusionRequest.BallId
			|| BallInstanceData->StarLevel != FusionRequest.SourceStarLevel)
		{
			return false;
		}
	}

	if (!DeckSubsystem->SetOwnedBallStarLevel(FusionRequest.SurvivorBallInstanceId, FusionRequest.ResultStarLevel))
	{
		return false;
	}

	for (const int32 BallInstanceId : FusionRequest.ConsumedBallInstanceIds)
	{
		if (BallInstanceId == FusionRequest.SurvivorBallInstanceId)
		{
			continue;
		}

		if (!DeckSubsystem->RemoveOwnedBall(BallInstanceId))
		{
			return false;
		}
	}

	return true;
}
