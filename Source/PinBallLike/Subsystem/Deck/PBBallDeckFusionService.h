// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "UObject/Object.h"
#include "PBBallDeckFusionService.generated.h"

class UPBBallDeckSubsystem;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDeckFusionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	int32 SurvivorBallInstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	TArray<int32> ConsumedBallInstanceIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	EPBBallDeckSlotType SurvivorSlotType = EPBBallDeckSlotType::Bench;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	int32 SurvivorSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	FName BallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	int32 SourceStarLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	int32 ResultStarLevel = 2;

	bool IsValid() const
	{
		return SurvivorBallInstanceId != INDEX_NONE
			&& ConsumedBallInstanceIds.Num() == RequiredFusionBallCount
			&& !BallId.IsNone()
			&& SourceStarLevel > 0
			&& SourceStarLevel < MaxFusionStarLevel
			&& ResultStarLevel > SourceStarLevel
			&& ResultStarLevel <= MaxFusionStarLevel;
	}

	static constexpr int32 RequiredFusionBallCount = 3;
	static constexpr int32 MaxFusionStarLevel = 3;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallDeckFusionBatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	TArray<FPBBallDeckFusionRequest> FusionRequests;

	bool IsValid() const
	{
		return !FusionRequests.IsEmpty();
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnBallFusionStarted, const FPBBallDeckFusionBatch&, FusionBatch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnBallFusionCompleted, const FPBBallDeckFusionBatch&, FusionBatch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnBallFusionCanceled, const FPBBallDeckFusionBatch&, FusionBatch);

UCLASS()
class PINBALLLIKE_API UPBBallDeckFusionService : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(UPBBallDeckSubsystem* InDeckSubsystem);

	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Fusion")
	FPBOnBallFusionStarted OnBallFusionStarted;

	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Fusion")
	FPBOnBallFusionCompleted OnBallFusionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Fusion")
	FPBOnBallFusionCanceled OnBallFusionCanceled;

	UFUNCTION(BlueprintCallable, Category = "BallDeck|Fusion")
	bool TryStartFusion();

	UFUNCTION(BlueprintCallable, Category = "BallDeck|Fusion")
	bool CompletePendingFusion();

	UFUNCTION(BlueprintCallable, Category = "BallDeck|Fusion")
	bool CancelPendingFusion();

	UFUNCTION(BlueprintPure, Category = "BallDeck|Fusion")
	bool HasPendingFusion() const;

	UFUNCTION(BlueprintPure, Category = "BallDeck|Fusion")
	FPBBallDeckFusionRequest GetPendingFusion() const;

	UFUNCTION(BlueprintPure, Category = "BallDeck|Fusion")
	FPBBallDeckFusionBatch GetPendingFusionBatch() const;

private:
	bool StartNextFusionBatch();
	bool BuildFusionRequests(TArray<FPBBallDeckFusionRequest>& OutFusionRequests) const;
	bool ValidateFusionRequests(const TArray<FPBBallDeckFusionRequest>& FusionRequests) const;
	bool ApplyFusionRequests(const TArray<FPBBallDeckFusionRequest>& FusionRequests) const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TArray<FPBBallDeckFusionRequest> PendingFusionRequests;
};
