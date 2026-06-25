// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBBallDeckSubsystem.generated.h"

class APBBallBase;

UENUM(BlueprintType)
enum class EPBDeploymentSlotRole : uint8
{
	None,
	Leader,
	Follower
};

UCLASS()
class PINBALLLIKE_API UPBBallDeckSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	EPBDeploymentSlotRole GetDeploymentRole(int32 SlotIndex) const;
	
	// Deployment Slot
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
	
	
protected:
	
	
private:
	void InitializeDeploymentSlots();

	static constexpr int32 MaxDeploymentSlotCount = 3;

	UPROPERTY()
	TArray<FPBBallDeckSlot> DeploymentSlots;
};
