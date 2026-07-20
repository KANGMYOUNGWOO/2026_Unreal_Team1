// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "PBCheatManager.generated.h"

class UGameInstance;
class UPBBallDeckSubsystem;
class UPBGameDataLoadSubsystem;

UCLASS()
class PINBALLLIKE_API UPBCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:

	UFUNCTION(exec)
	void AddDefaultBall();

	UFUNCTION(exec)
	void AddBenchBall(FName BallId);

	UFUNCTION(exec)
	void GoScene(int32 SceneIndex);

	UFUNCTION(exec)
	void GoMainMenu();

	UFUNCTION(exec)
	void GoBattle();

	UFUNCTION(exec)
	void GoShop();

	UFUNCTION(exec)
	void PrintAsyncLoadState() const;

	UFUNCTION(exec)
	void DamageBoss(int32 DamageAmount);

	UFUNCTION(exec)
	void DamageBall();

	UFUNCTION(exec)
	void RegenMana();

	UFUNCTION(exec)
	void ShowBallStatEffect();

	UFUNCTION(exec)
	void ShowPopup();

	UFUNCTION(exec)
	void DamageGolemHand(FName HandName, int32 DamageAmount);

	UFUNCTION(exec)
	void AddBossGroggy(int32 GroggyAmount);

	UFUNCTION(exec)
	void BumperStatus() const;

	UFUNCTION(exec)
	void BumperCharge(FName PositionName, int32 Count);

	UFUNCTION(exec)
	void BumperComplete(FName PositionName);

	UFUNCTION(exec)
	void BumperReset(FName PositionName);

private:
	UGameInstance* GetCheatGameInstance() const;
	UPBBallDeckSubsystem* GetBallDeckSubsystem() const;
	UPBGameDataLoadSubsystem* GetGameDataLoadSubsystem() const;
	void RequestPlacedBallAssetReload();
	void HandleCheatPlacedBallGameplayAssetsLoaded();
};
