
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBPlayerDataSubsystem.generated.h"

UENUM(BlueprintType)
enum class EPBBumperSlotType : uint8
{
	Rebound UMETA(DisplayName = "Rebound"),
	Side UMETA(DisplayName = "Side"),
	Special UMETA(DisplayName = "Special")
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

};
