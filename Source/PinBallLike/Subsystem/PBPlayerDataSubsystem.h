
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBPlayerDataSubsystem.generated.h"

UENUM(BlueprintType)
enum class EPBBumperSlotType : uint8
{
	Rebound = 0 UMETA(DisplayName = "Rebound"),
	Side = 1 UMETA(DisplayName = "Side"),
	Special = 2 UMETA(DisplayName = "Special"),
	// 기존 열거형 값을 유지하기 위해 새 Top 슬롯은 마지막에 추가한다.
	Top = 3 UMETA(DisplayName = "Top")
};

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
	TMap<EPBBumperSlotType, FName> EquippedBumperRowIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleLaunchCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerData|Battle", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 InitialBattleShiftCount = 3;

	UPROPERTY(Transient)
	int32 CurrentBossIndex = 0;

};
