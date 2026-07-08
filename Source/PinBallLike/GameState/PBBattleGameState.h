// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "PBBattleGameState.generated.h"

class APBBossSpawnController;
class APBBumperSpawnController;
enum class EPBBattlePreparationType : uint8;
struct FPBBattleBossDeadMessage;
struct FPBBattlePartyAllBallsDeadMessage;
struct FPBBattlePartyLaunchedMessage;
struct FPBBattlePartyLaunchRequestedMessage;
struct FPBBattlePartyShiftRequestedMessage;
struct FPBBattlePreparationCompletedMessage;

UENUM(BlueprintType)
enum class EPBBattleLevelPhase : uint8
{
	None UMETA(DisplayName = "None"),
	LevelPreparing UMETA(DisplayName = "Level Preparing"),
	BossIntro UMETA(DisplayName = "Boss Intro"),
	BallDeployment UMETA(DisplayName = "Ball Deployment"),
	Combat UMETA(DisplayName = "Combat"),
	BossDead UMETA(DisplayName = "Boss Dead"),
	Reward UMETA(DisplayName = "Reward")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBattleLevelPhaseChangedSignature,
	EPBBattleLevelPhase, PreviousPhase,
	EPBBattleLevelPhase, NewPhase);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBattleGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APBBattleGameState();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	void StartBattleLevelFlow();

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	void SetBattleLevelPhase(EPBBattleLevelPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	void CompleteLevelPreparing();

	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	EPBBattleLevelPhase GetBattleLevelPhase() const { return CurrentPhase; }

	UPROPERTY(BlueprintAssignable, Category = "Battle|Flow")
	FPBBattleLevelPhaseChangedSignature OnBattleLevelPhaseChanged;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleLevelPreparing();
	virtual void HandleLevelPreparing_Implementation();

	void PrepareBumpers();

	void PrepareBalls();

	void PrepareBoss();

	void HandleBallGameplayAssetsLoaded();

	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void HandlePreparationCompletedMessage(FGameplayTag Channel, const FPBBattlePreparationCompletedMessage& Message);
	void HandleBossDeadMessage(FGameplayTag Channel, const FPBBattleBossDeadMessage& Message);
	void HandlePartyLaunchRequestedMessage(FGameplayTag Channel, const FPBBattlePartyLaunchRequestedMessage& Message);
	void HandlePartyLaunchedMessage(FGameplayTag Channel, const FPBBattlePartyLaunchedMessage& Message);
	void HandlePartyAllBallsDeadMessage(FGameplayTag Channel, const FPBBattlePartyAllBallsDeadMessage& Message);
	void HandlePartyShiftRequestedMessage(FGameplayTag Channel, const FPBBattlePartyShiftRequestedMessage& Message);
	void ResetPreparationState();
	void MarkPreparationCompleted(EPBBattlePreparationType PreparationType, bool bSuccess);

	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleBossIntro();
	virtual void HandleBossIntro_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleBallDeployment();
	virtual void HandleBallDeployment_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleBattle();
	virtual void HandleBattle_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleBossDead();
	virtual void HandleBossDead_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Battle|Flow")
	void HandleReward();
	virtual void HandleReward_Implementation();

private:
	void HandleCurrentPhase();
	void InitializeBattleLaunchCount();
	bool CanLaunchBattleParty() const;
	bool NotifyBattlePartyLaunched();
	void NotifyBattlePartyAllBallsDead();

	UPROPERTY(EditAnywhere, Category = "Battle|Flow")
	bool bStartFlowOnBeginPlay = true;

	UPROPERTY()
	TObjectPtr<APBBumperSpawnController> BumperSpawnController;

	UPROPERTY()
	TObjectPtr<APBBossSpawnController> BossSpawnController;

	FGameplayMessageListenerHandle PreparationCompletedListenerHandle;
	FGameplayMessageListenerHandle BossDeadListenerHandle;
	FGameplayMessageListenerHandle PartyLaunchRequestedListenerHandle;
	FGameplayMessageListenerHandle PartyLaunchedListenerHandle;
	FGameplayMessageListenerHandle PartyAllBallsDeadListenerHandle;
	FGameplayMessageListenerHandle PartyShiftRequestedListenerHandle;

	bool bBumperPrepared = false;
	bool bBallPrepared = false;
	bool bBossPrepared = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Flow")
	EPBBattleLevelPhase CurrentPhase = EPBBattleLevelPhase::None;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Flow")
	int32 RemainingBattleLaunchCount = 0;
};
