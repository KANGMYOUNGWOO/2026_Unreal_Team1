// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Ball/PBBallItemViewData.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PinBallLike/Struct/Deck/PBBallInstanceData.h"
#include "PinBallLike/Struct/Party/PBPartyTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBBallDeckSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBOnDeploymentSlotChanged, int32, SlotIndex, int32, BallInstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOnDeploymentSlotsReordered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOnDeploymentSlotsRotated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBOnBenchSlotChanged, int32, SlotIndex, int32, BallInstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOnBenchSlotsSwapped);

UCLASS()
class PINBALLLIKE_API UPBBallDeckSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	int32 AddOwnedBall(int32 BallId, int32 StarLevel = 1);
	const FPBBallInstanceData* GetOwnedBallData(int32 BallInstanceId) const;
	bool HasOwnedBall(int32 BallInstanceId) const;
	bool AddNewBallToDeck(int32 BallId, int32 StarLevel = 1);
	bool BuildBallItemViewData(int32 BallInstanceId, EPBBallDeckSlotType SourceSlotType, int32 SourceSlotIndex, FPBBallItemViewData& OutViewData) const;

	UFUNCTION(BlueprintCallable, Category = "BallDeck|DragDrop")
	bool MoveBallBetweenSlots(EPBBallDeckSlotType SourceSlotType, int32 SourceSlotIndex, EPBBallDeckSlotType TargetSlotType, int32 TargetSlotIndex);

#pragma region Deployment Slot

public:
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Deployment")
	FPBOnDeploymentSlotChanged OnDeploymentSlotChanged;
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Deployment")
	FPBOnDeploymentSlotsReordered OnDeploymentSlotsReordered;
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Deployment")
	FPBOnDeploymentSlotsRotated OnDeploymentSlotsRotated;
	
	EPBBallPartyRole GetDeploymentRole(int32 SlotIndex) const;
	bool SetDeploymentSlot(int32 SlotIndex, int32 BallInstanceId);
	bool ClearDeploymentSlot(int32 SlotIndex);
	bool SwapDeploymentSlots(int32 FirstIndex, int32 SecondIndex);
	bool IsDeploymentSlotValid(int32 SlotIndex) const;
	bool IsDeploymentSlotOccupied(int32 SlotIndex) const;
	int32 FindEmptyDeploymentSlot() const;
	int32 GetDeploymentSlotBallInstanceId(int32 SlotIndex) const;
	int32 GetLeaderBallInstanceId() const;
	TArray<int32> GetDeploymentBallInstanceIds() const;
	TArray<int32> GetFollowerBallInstanceIds() const;
	int32 GetDeploymentBallCount() const;
	bool HasLeaderBall() const;
	bool CanBuildDeploymentParty() const;
	void CompactDeploymentSlots();
	bool RotateDeploymentSlots();

private:
	void InitializeDeploymentSlots();
	TArray<int32> CaptureDeploymentSlotBallInstanceIds() const;
	bool CompactDeploymentSlotsInternal();
	void BroadcastDeploymentSlotChange(const TArray<int32>& PreviousBallInstanceIds);
	void ClearBallInstanceFromSlots(int32 BallInstanceId);

	static constexpr int32 MaxDeploymentSlotCount = 3;

	UPROPERTY()
	TArray<FPBBallDeckSlot> DeploymentSlots;

#pragma endregion

#pragma region Bench

public:
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Bench")
	FPBOnBenchSlotChanged OnBenchSlotChanged;
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Bench")
	FPBOnBenchSlotsSwapped OnBenchSlotsSwapped;

	bool SetBenchSlot(int32 SlotIndex, int32 BallInstanceId);
	bool ClearBenchSlot(int32 SlotIndex);
	bool SwapBenchSlots(int32 FirstIndex, int32 SecondIndex);
	bool IsBenchSlotValid(int32 SlotIndex) const;
	bool IsBenchSlotOccupied(int32 SlotIndex) const;
	bool HasEmptyBenchSlot() const;
	int32 FindEmptyBenchSlot() const;
	bool AddBenchBall(int32 BallInstanceId);
	bool AddNewBallToBench(int32 BallId, int32 StarLevel = 1);
	int32 GetBenchBallInstanceId(int32 SlotIndex) const;
	TArray<int32> GetBenchBallInstanceIds() const;
	int32 GetBenchBallCount() const;

private:
	void InitializeBenchSlots();

	static constexpr int32 MaxBenchSlotCount = 9;

	UPROPERTY()
	TArray<FPBBallDeckSlot> BenchSlots;

#pragma endregion

private:
	FPBBallDeckSlot* GetMutableDeckSlot(EPBBallDeckSlotType SlotType, int32 SlotIndex);
	void BroadcastDeckSlotChanged(EPBBallDeckSlotType SlotType, int32 SlotIndex, int32 BallInstanceId);

	UPROPERTY()
	TMap<int32, FPBBallInstanceData> OwnedBallDataMap;

	int32 NextBallInstanceId = 1;
};
