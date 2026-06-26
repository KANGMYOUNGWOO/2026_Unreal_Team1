// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PinBallLike/Struct/Party/PBPartyTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBBallDeckSubsystem.generated.h"

class APBBallBase;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBOnDeploymentSlotChanged, int32, SlotIndex, APBBallBase*, Ball);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOnDeploymentSlotsReordered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOnDeploymentSlotsRotated);

UCLASS()
class PINBALLLIKE_API UPBBallDeckSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Deployment")
	FPBOnDeploymentSlotChanged OnDeploymentSlotChanged;
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Deployment")
	FPBOnDeploymentSlotsReordered OnDeploymentSlotsReordered;
	UPROPERTY(BlueprintAssignable, Category = "BallDeck|Deployment")
	FPBOnDeploymentSlotsRotated OnDeploymentSlotsRotated;
	
	
	// Deployment Slot
	EPBBallPartyRole GetDeploymentRole(int32 SlotIndex) const;
	bool SetDeploymentSlot(int32 SlotIndex, APBBallBase* Ball);
	bool ClearDeploymentSlot(int32 SlotIndex);
	bool SwapDeploymentSlots(int32 FirstIndex, int32 SecondIndex);
	bool IsDeploymentSlotValid(int32 SlotIndex) const;
	bool IsDeploymentSlotOccupied(int32 SlotIndex) const;
	APBBallBase* GetDeploymentSlotBall(int32 SlotIndex) const;
	APBBallBase* GetLeaderBall() const;
	TArray<APBBallBase*> GetDeploymentBalls() const;
	TArray<APBBallBase*> GetFollowerBalls() const;
	int32 GetDeploymentBallCount() const;
	bool HasLeaderBall() const;
	bool CanBuildDeploymentParty() const;
	void CompactDeploymentSlots();
	bool RotateDeploymentSlots();

	UFUNCTION(BlueprintCallable, Category = "BallDeck|Test")
	bool TestSnakeBall(TSubclassOf<APBBallBase> BallClass);
	
protected:
	
	
private:
	void InitializeDeploymentSlots();
	TArray<APBBallBase*> CaptureDeploymentSlotBalls() const;
	bool CompactDeploymentSlotsInternal();
	void BroadcastDeploymentSlotChange(const TArray<APBBallBase*>& PreviousBalls);

	static constexpr int32 MaxDeploymentSlotCount = 3;

	UPROPERTY()
	TArray<FPBBallDeckSlot> DeploymentSlots;
};
