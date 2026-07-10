// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PBBattleGameMode.generated.h"

class APBBossSpawner;
class APBBumperSpawner;
enum class EPBBattlePreparationType : uint8;
struct FPBBattleBossDeadMessage;
struct FPBBattlePartyAllBallsDeadMessage;
struct FPBBattlePartyLaunchedMessage;
struct FPBBattlePartyLaunchRequestedMessage;
struct FPBBattlePartyShiftRequestedMessage;
struct FPBBattlePreparationCompletedMessage;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBattleGameMode : public AGameModeBase
{
	GENERATED_BODY()

#pragma region Lifecycle

public:
	APBBattleGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma endregion

#pragma region PhaseFlow

public:
	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	bool CanLaunchBattleParty() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleReward();
	virtual void HandleReward_Implementation();

private:
	APBBattleGameState* GetBattleGameState() const;
	
	void SetBattleLevelPhase(EPBBattleLevelPhase NewPhase);
	void HandleCurrentPhase();
	void InitializeBattleLaunchCount();
	void EnterBattleDataLoad();
	void TryStartLevelPreparing();
	void EnterLevelPreparing();
	void TryStartBossInfo();
	void EnterBossIntro();
	void EnterBallDeployment();
	void EnterBattle();
	void EnterBossDead();

#pragma endregion
	
#pragma region DataLoad
	
private:
	void LoadBumpers();
	void LoadBalls();
	void LoadBoss();
	void HandleBumperDataLoaded();
	void HandleBallDataLoaded();
	void HandleBossDataLoaded();
	void MarkDataLoaded(EPBBattlePreparationType PreparationType, bool bSuccess);
	bool IsBattleDataLoaded() const;
	
	bool bBumperDataLoaded = false;
	bool bBallDataLoaded = false;
	bool bBossDataLoaded = false;

#pragma endregion 
	
#pragma region LevelPreparing

private:
	void ResetBattlePreparationState();
	APBBumperSpawner* FindBumperSpawner();
	APBBossSpawner* FindBossSpawner();
	void PrepareBumpers();
	void PrepareBoss();
	
	UE_DEPRECATED(6.0, "This function is deprecated. Use NewFunction instead.")
	void MarkPreparationCompleted(EPBBattlePreparationType PreparationType, bool bSuccess);
	bool IsBattlePreparationCompleted() const;

	bool bBumperPrepared = false;
	bool bBossPrepared = false;
#pragma endregion

#pragma region MessageHandler

private:
	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void HandlePreparationCompletedMessage(FGameplayTag Channel, const FPBBattlePreparationCompletedMessage& Message);
	void HandleBossDeadMessage(FGameplayTag Channel, const FPBBattleBossDeadMessage& Message);
	void HandlePartyLaunchRequestedMessage(FGameplayTag Channel, const FPBBattlePartyLaunchRequestedMessage& Message);
	void HandlePartyLaunchedMessage(FGameplayTag Channel, const FPBBattlePartyLaunchedMessage& Message);
	void HandlePartyAllBallsDeadMessage(FGameplayTag Channel, const FPBBattlePartyAllBallsDeadMessage& Message);
	void HandlePartyShiftRequestedMessage(FGameplayTag Channel, const FPBBattlePartyShiftRequestedMessage& Message);

	FGameplayMessageListenerHandle PreparationCompletedListenerHandle;
	FGameplayMessageListenerHandle BossDeadListenerHandle;
	FGameplayMessageListenerHandle PartyLaunchRequestedListenerHandle;
	FGameplayMessageListenerHandle PartyLaunchedListenerHandle;
	FGameplayMessageListenerHandle PartyAllBallsDeadListenerHandle;
	FGameplayMessageListenerHandle PartyShiftRequestedListenerHandle;

#pragma endregion

private:
	UPROPERTY()
	TObjectPtr<APBBumperSpawner> BumperSpawner;

	UPROPERTY()
	TObjectPtr<APBBossSpawner> BossSpawner;

	bool bStartPlayCompleted = false;
};
