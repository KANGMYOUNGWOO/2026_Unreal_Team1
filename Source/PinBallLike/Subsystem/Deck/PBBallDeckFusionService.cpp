// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckFusionService.h"

#include "PBBallDeckSubsystem.h"
#include "PinBallLike/Struct/Deck/PBBallInstanceData.h"

void UPBBallDeckFusionService::Initialize(UPBBallDeckSubsystem* InDeckSubsystem)
{
	DeckSubsystem = InDeckSubsystem;
	PendingFusionRequests.Reset();
}

bool UPBBallDeckFusionService::TryStartFusion()
{
	if (HasPendingFusion())
	{
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("UPBBallDeckFusionService TryStartFusion"));
	return StartNextFusionBatch();
}

bool UPBBallDeckFusionService::StartNextFusionBatch()
{
	TArray<FPBBallDeckFusionRequest> FusionRequests;
	if (!BuildFusionRequests(FusionRequests))
	{
		return false;
	}

	PendingFusionRequests = FusionRequests;

	FPBBallDeckFusionBatch FusionBatch;
	FusionBatch.FusionRequests = PendingFusionRequests;
	OnBallFusionStarted.Broadcast(FusionBatch);
	return true;
}

bool UPBBallDeckFusionService::CompletePendingFusion()
{
	if (!HasPendingFusion())
	{
		return false;
	}

	const TArray<FPBBallDeckFusionRequest> CompletedFusionRequests = PendingFusionRequests;
	if (!ApplyFusionRequests(CompletedFusionRequests))
	{
		return false;
	}

	PendingFusionRequests.Reset();

	FPBBallDeckFusionBatch FusionBatch;
	FusionBatch.FusionRequests = CompletedFusionRequests;
	OnBallFusionCompleted.Broadcast(FusionBatch);

	StartNextFusionBatch();
	return true;
}

bool UPBBallDeckFusionService::CancelPendingFusion()
{
	if (!HasPendingFusion())
	{
		return false;
	}

	FPBBallDeckFusionBatch FusionBatch;
	FusionBatch.FusionRequests = PendingFusionRequests;

	PendingFusionRequests.Reset();
	OnBallFusionCanceled.Broadcast(FusionBatch);
	return true;
}

bool UPBBallDeckFusionService::HasPendingFusion() const
{
	return !PendingFusionRequests.IsEmpty();
}

FPBBallDeckFusionRequest UPBBallDeckFusionService::GetPendingFusion() const
{
	return PendingFusionRequests.IsValidIndex(0)
		? PendingFusionRequests[0]
		: FPBBallDeckFusionRequest();
}

FPBBallDeckFusionBatch UPBBallDeckFusionService::GetPendingFusionBatch() const
{
	FPBBallDeckFusionBatch FusionBatch;
	FusionBatch.FusionRequests = PendingFusionRequests;
	return FusionBatch;
}

bool UPBBallDeckFusionService::BuildFusionRequests(TArray<FPBBallDeckFusionRequest>& OutFusionRequests) const
{
	OutFusionRequests.Reset();
	if (!DeckSubsystem)
	{
		return false;
	}

	TMap<int64, TArray<int32>> FusionCandidatesByKey;
	TArray<int64> FusionCandidateKeys;
	const TArray<int32> PlacedBallInstanceIds = DeckSubsystem->GetAllPlacedBallInstanceIds();
	for (const int32 BallInstanceId : PlacedBallInstanceIds)
	{
		const FPBBallInstanceData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
		if (!BallInstanceData || !BallInstanceData->IsValid())
		{
			continue;
		}

		if (BallInstanceData->StarLevel >= FPBBallDeckFusionRequest::MaxFusionStarLevel)
		{
			continue;
		}

		const int64 CandidateKey = (static_cast<int64>(BallInstanceData->BallId) << 32)
			| static_cast<uint32>(BallInstanceData->StarLevel);
		if (!FusionCandidatesByKey.Contains(CandidateKey))
		{
			FusionCandidateKeys.Add(CandidateKey);
		}

		TArray<int32>& CandidateBallInstanceIds = FusionCandidatesByKey.FindOrAdd(CandidateKey);
		CandidateBallInstanceIds.Add(BallInstanceId);
	}

	for (const int64 CandidateKey : FusionCandidateKeys)
	{
		const TArray<int32>* CandidateBallInstanceIdsPtr = FusionCandidatesByKey.Find(CandidateKey);
		if (!CandidateBallInstanceIdsPtr)
		{
			continue;
		}

		const TArray<int32>& CandidateBallInstanceIds = *CandidateBallInstanceIdsPtr;
		const int32 FusionRequestCount = CandidateBallInstanceIds.Num() / FPBBallDeckFusionRequest::RequiredFusionBallCount;
		for (int32 RequestIndex = 0; RequestIndex < FusionRequestCount; ++RequestIndex)
		{
			const int32 FirstConsumedIndex = RequestIndex * FPBBallDeckFusionRequest::RequiredFusionBallCount;
			const int32 SurvivorBallInstanceId = CandidateBallInstanceIds[FirstConsumedIndex];
			const FPBBallInstanceData* SurvivorBallInstanceData = DeckSubsystem->GetOwnedBallData(SurvivorBallInstanceId);
			if (!SurvivorBallInstanceData || !SurvivorBallInstanceData->IsValid())
			{
				continue;
			}

			if (SurvivorBallInstanceData->StarLevel >= FPBBallDeckFusionRequest::MaxFusionStarLevel)
			{
				continue;
			}

			FPBBallDeckSlot SurvivorLocation;
			if (!DeckSubsystem->FindBallLocation(SurvivorBallInstanceId, SurvivorLocation))
			{
				continue;
			}

			FPBBallDeckFusionRequest FusionRequest;
			FusionRequest.SurvivorBallInstanceId = SurvivorBallInstanceId;
			FusionRequest.ConsumedBallInstanceIds = {
				CandidateBallInstanceIds[FirstConsumedIndex],
				CandidateBallInstanceIds[FirstConsumedIndex + 1],
				CandidateBallInstanceIds[FirstConsumedIndex + 2]
			};
			FusionRequest.SurvivorSlotType = SurvivorLocation.SlotType;
			FusionRequest.SurvivorSlotIndex = SurvivorLocation.SlotIndex;
			FusionRequest.BallId = SurvivorBallInstanceData->BallId;
			FusionRequest.SourceStarLevel = SurvivorBallInstanceData->StarLevel;
			FusionRequest.ResultStarLevel = SurvivorBallInstanceData->StarLevel + 1;
			OutFusionRequests.Add(FusionRequest);
		}
	}

	return !OutFusionRequests.IsEmpty() && ValidateFusionRequests(OutFusionRequests);
}

bool UPBBallDeckFusionService::ValidateFusionRequests(const TArray<FPBBallDeckFusionRequest>& FusionRequests) const
{
	if (!DeckSubsystem || FusionRequests.IsEmpty())
	{
		return false;
	}

	TSet<int32> ConsumedBallInstanceIds;
	for (const FPBBallDeckFusionRequest& FusionRequest : FusionRequests)
	{
		if (!FusionRequest.IsValid())
		{
			return false;
		}

		for (const int32 BallInstanceId : FusionRequest.ConsumedBallInstanceIds)
		{
			if (ConsumedBallInstanceIds.Contains(BallInstanceId))
			{
				return false;
			}

			const FPBBallInstanceData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
			if (!BallInstanceData
				|| !BallInstanceData->IsValid()
				|| BallInstanceData->BallId != FusionRequest.BallId
				|| BallInstanceData->StarLevel != FusionRequest.SourceStarLevel)
			{
				return false;
			}

			ConsumedBallInstanceIds.Add(BallInstanceId);
		}
	}

	return true;
}

bool UPBBallDeckFusionService::ApplyFusionRequests(const TArray<FPBBallDeckFusionRequest>& FusionRequests) const
{
	if (!ValidateFusionRequests(FusionRequests))
	{
		return false;
	}

	for (const FPBBallDeckFusionRequest& FusionRequest : FusionRequests)
	{
		if (!DeckSubsystem->SetOwnedBallStarLevel(FusionRequest.SurvivorBallInstanceId, FusionRequest.ResultStarLevel))
		{
			return false;
		}
	}

	for (const FPBBallDeckFusionRequest& FusionRequest : FusionRequests)
	{
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
	}

	return true;
}
