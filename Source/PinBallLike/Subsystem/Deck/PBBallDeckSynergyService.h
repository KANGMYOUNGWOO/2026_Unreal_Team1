#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Ball/PBBallClassType.h"
#include "PinBallLike/Struct/Ball/PBBallRaceType.h"
#include "PinBallLike/Struct/Synergy/PBSynergyState.h"
#include "UObject/Object.h"
#include "PBBallDeckSynergyService.generated.h"

class UPBBallDeckSubsystem;
class UPBTableDataSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FPBOnSynergyStatesChanged, const TArray<FPBSynergyState>&);

UCLASS()
class PINBALLLIKE_API UPBBallDeckSynergyService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UPBBallDeckSubsystem* InDeckSubsystem);
	void Deinitialize();

	void RefreshSynergyStatesFromPlacedDeck();
	const TArray<FPBSynergyState>& GetActiveSynergyStates() const { return ActiveSynergyStates; }

	bool BuildSynergyStatesFromBallIds(
		const TArray<FName>& BallIds,
		TArray<FPBSynergyState>& OutStates) const;

	bool BuildSynergyStatesFromBallInstanceIds(
		const TArray<int32>& BallInstanceIds,
		TArray<FPBSynergyState>& OutStates) const;

	FPBOnSynergyStatesChanged OnSynergyStatesChanged;

private:
	void BindDeckEvents();
	void UnbindDeckEvents();
	const UPBTableDataSubsystem* GetTableDataSubsystem() const;
	static FName GetRaceSynergyId(EPBBallRaceType RaceType);
	static FName GetClassSynergyId(EPBBallClassType ClassType);

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeckChanged();

	UFUNCTION()
	void HandleBenchSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleBallSold(int32 BallInstanceId, FName BallId, int32 SellPrice);

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TArray<FPBSynergyState> ActiveSynergyStates;

	bool bDeckEventsBound = false;
};
