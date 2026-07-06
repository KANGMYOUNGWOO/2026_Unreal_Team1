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
	int32 BallId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	int32 SourceStarLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|Fusion")
	int32 ResultStarLevel = 2;

	bool IsValid() const
	{
		return SurvivorBallInstanceId != INDEX_NONE
			&& ConsumedBallInstanceIds.Num() == RequiredFusionBallCount
			&& BallId != 0
			&& SourceStarLevel > 0
			&& ResultStarLevel > SourceStarLevel;
	}

	static constexpr int32 RequiredFusionBallCount = 3;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnBallFusionStarted, const FPBBallDeckFusionRequest&, FusionRequest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnBallFusionCompleted, const FPBBallDeckFusionRequest&, FusionRequest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnBallFusionCanceled, const FPBBallDeckFusionRequest&, FusionRequest);

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

private:
	bool BuildNextFusionRequest(FPBBallDeckFusionRequest& OutFusionRequest) const;
	bool ApplyFusionRequest(const FPBBallDeckFusionRequest& FusionRequest) const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	FPBBallDeckFusionRequest PendingFusion;
};
