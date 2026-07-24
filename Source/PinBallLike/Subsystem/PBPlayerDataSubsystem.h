
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBPlayerDataSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBOnGoldChanged, int32, NewGold);

UCLASS()
class PINBALLLIKE_API UPBPlayerDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region Battle
	UFUNCTION(BlueprintPure, Category = "PlayerData|Battle")
	int32 GetInitialBattleLaunchCount() const { return InitialBattleLaunchCount; }

	UFUNCTION(BlueprintPure, Category = "PlayerData|Battle")
	int32 GetInitialBattleShiftCount() const { return InitialBattleShiftCount; }

	UFUNCTION(BlueprintPure, Category = "PlayerData|Battle")
	float GetBattleDashCooldownSeconds() const { return BattleDashCooldownSeconds; }

	UFUNCTION(BlueprintPure, Category = "PlayerData|Battle")
	int32 GetCurrentBossIndex() const { return CurrentBossIndex; }

	bool SetCurrentBossIndex(int32 NewBossIndex, int32 BossCount);

	void AdvanceBossProgress(int32 BossCount);

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Run")
	void ResetRunData();
#pragma endregion

#pragma region Bumper
	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool EquipBumperAtSlot(EPBBumperEquipSlot EquipSlot, FName BumperRowId);

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool UnequipBumperAtSlot(EPBBumperEquipSlot EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool MoveEquippedBumperBetweenSlots(
		EPBBumperEquipSlot SourceSlot,
		EPBBumperEquipSlot TargetSlot);

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	bool GetEquippedBumperAtSlot(EPBBumperEquipSlot EquipSlot, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	TArray<FPBEquippedBumperSlot> GetEquippedBumperSlots() const;

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	bool IsBumperEquippedInAnotherSlot(FName BumperRowId, EPBBumperEquipSlot TargetSlot) const;

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool EquipBumper(EPBBumperSlotType SlotType, FName BumperRowId);

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool UnequipBumper(EPBBumperSlotType SlotType);

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	bool GetEquippedBumper(EPBBumperSlotType SlotType, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	TArray<FName> GetEquippedBumperRowIds() const;
#pragma endregion 

	UFUNCTION(BlueprintPure, Category = "PlayerData")
	int32 GetCurrentGold() const { return Gold; }

	UFUNCTION(BlueprintCallable, Category = "PlayerData")
	void GainGold(int32 Amount);
	
	UFUNCTION(BlueprintCallable, Category = "PlayerData")
	void SpendGold(int32 Amount);

	UPROPERTY(BlueprintAssignable, Category = "PlayerData|Gold")
	FPBOnGoldChanged OnGoldChanged;
private:
#if WITH_DEV_AUTOMATION_TESTS
	friend class FPBBumperLoadoutTransactionTest;
#endif

	void InitializeDefaultBumpers();
	bool LoadBumperLoadout();
	bool CommitBumperLoadout(TMap<EPBBumperEquipSlot, FName>&& CandidateLoadout);
	bool SaveBumperLoadout(const TMap<EPBBumperEquipSlot, FName>& Loadout) const;
	bool WriteBumperLoadout(const TArray<FPBEquippedBumperSlot>& EquippedSlots) const;
	static TArray<FPBEquippedBumperSlot> BuildEquippedBumperSlots(
		const TMap<EPBBumperEquipSlot, FName>& Loadout);
	bool ValidateBumperForSlot(EPBBumperEquipSlot EquipSlot, FName BumperRowId) const;
	void SanitizeEquippedBumpers();

	UFUNCTION()
	void HandleStartupGameDataLoaded();

	UPROPERTY(Transient)
	TMap<EPBBumperEquipSlot, FName> EquippedBumperRowIds;

	bool bBumperPersistenceInitialized = false;
	bool bLoadedBumperLoadoutNeedsResave = false;

#if WITH_DEV_AUTOMATION_TESTS
	TFunction<bool(const TArray<FPBEquippedBumperSlot>&)> BumperLoadoutWriterOverride;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 Gold = 1000;

	UPROPERTY(Transient)
	int32 InitialGold = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleLaunchCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleShiftCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BattleDashCooldownSeconds = 5.0f;

	UPROPERTY(Transient)
	int32 CurrentBossIndex = 0;

};
