
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBPlayerDataSubsystem.generated.h"

UCLASS()
class PINBALLLIKE_API UPBPlayerDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

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
	// TODO: SaveGame 연동 전까지 사용하는 임시 기본 장착값.
	void InitializeDefaultBumpersForTest();

	UPROPERTY(Transient)
	TMap<EPBBumperEquipSlot, FName> EquippedBumperRowIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleLaunchCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleShiftCount = 3;

	UPROPERTY(Transient)
	int32 CurrentBossIndex = 0;

};
