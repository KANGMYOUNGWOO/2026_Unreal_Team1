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
	void BallDamage();

private:
	UGameInstance* GetCheatGameInstance() const;
	UPBBallDeckSubsystem* GetBallDeckSubsystem() const;
	UPBGameDataLoadSubsystem* GetGameDataLoadSubsystem() const;
	void RequestPlacedBallAssetReload();
	void HandleCheatPlacedBallGameplayAssetsLoaded();
};
