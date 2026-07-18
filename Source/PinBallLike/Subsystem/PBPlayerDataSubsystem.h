
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBPlayerDataSubsystem.generated.h"

/** 전투 진행값과 플레이어의 범퍼 장착 구성을 소유하고 저장하는 게임 인스턴스 서비스입니다. */
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
	int32 GetCurrentBossIndex() const { return CurrentBossIndex; }

	void AdvanceBossProgress(int32 BossCount);
#pragma endregion

#pragma region Bumper
	/** 지정한 좌우 물리 슬롯 하나에 범퍼를 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool EquipBumperAtSlot(EPBBumperEquipSlot EquipSlot, FName BumperRowId);

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool UnequipBumperAtSlot(EPBBumperEquipSlot EquipSlot);

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	bool GetEquippedBumperAtSlot(EPBBumperEquipSlot EquipSlot, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	TArray<FPBEquippedBumperSlot> GetEquippedBumperSlots() const;

	/** 같은 Row가 대상 슬롯 외의 위치에 이미 장착되어 있는지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	bool IsBumperEquippedInAnotherSlot(FName BumperRowId, EPBBumperEquipSlot TargetSlot) const;

	/** 기존 카테고리 기반 호출을 보존합니다. 중복 방지를 위해 카테고리의 대표 위치 하나만 변경합니다. */
	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool EquipBumper(EPBBumperSlotType SlotType, FName BumperRowId);

	UFUNCTION(BlueprintCallable, Category = "PlayerData|Bumper")
	bool UnequipBumper(EPBBumperSlotType SlotType);

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	bool GetEquippedBumper(EPBBumperSlotType SlotType, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "PlayerData|Bumper")
	TArray<FName> GetEquippedBumperRowIds() const;
#pragma endregion 

private:
	void InitializeDefaultBumpers();
	bool LoadBumperLoadout();
	bool SaveBumperLoadout() const;
	bool ValidateBumperForSlot(EPBBumperEquipSlot EquipSlot, FName BumperRowId) const;
	void SanitizeEquippedBumpers();

	UFUNCTION()
	void HandleStartupGameDataLoaded();

	UPROPERTY(Transient)
	TMap<EPBBumperEquipSlot, FName> EquippedBumperRowIds;

	bool bBumperPersistenceInitialized = false;
	bool bLoadedBumperLoadoutNeedsResave = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleLaunchCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleShiftCount = 3;

	UPROPERTY(Transient)
	int32 CurrentBossIndex = 0;

};
