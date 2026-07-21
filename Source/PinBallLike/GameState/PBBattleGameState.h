// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PBBattleGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBattleLevelPhaseChangedSignature,
	EPBBattleLevelPhase, PreviousPhase,
	EPBBattleLevelPhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBattleLaunchCountChangedSignature,
	int32, PreviousCount,
	int32, NewCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBBattleShiftCountChangedSignature,
	int32, PreviousCount,
	int32, NewCount);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBattleGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APBBattleGameState();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	void SetBattleLevelPhase(EPBBattleLevelPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	void SetRemainingBattleLaunchCount(int32 NewRemainingBattleLaunchCount);

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	bool ConsumeBattleLaunchCount();

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	void SetRemainingBattleShiftCount(int32 NewRemainingBattleShiftCount);

	UFUNCTION(BlueprintCallable, Category = "Battle|Flow")
	bool ConsumeBattleShiftCount();

	UFUNCTION(BlueprintCallable, Category = "Battle|Dash")
	void SetBattleDashCooldownSeconds(float NewBattleDashCooldownSeconds);

	UFUNCTION(BlueprintCallable, Category = "Battle|Dash")
	bool ConsumeBattleDash();

	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	EPBBattleLevelPhase GetBattleLevelPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	int32 GetRemainingBattleLaunchCount() const { return RemainingBattleLaunchCount; }

	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	bool HasRemainingBattleLaunchCount() const { return RemainingBattleLaunchCount > 0; }

	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	int32 GetRemainingBattleShiftCount() const { return RemainingBattleShiftCount; }

	UFUNCTION(BlueprintPure, Category = "Battle|Flow")
	bool HasRemainingBattleShiftCount() const { return RemainingBattleShiftCount > 0; }

	UFUNCTION(BlueprintPure, Category = "Battle|Dash")
	float GetBattleDashCooldownSeconds() const { return BattleDashCooldownSeconds; }

	UFUNCTION(BlueprintPure, Category = "Battle|Dash")
	float GetRemainingBattleDashCooldown() const;

	UFUNCTION(BlueprintPure, Category = "Battle|Dash")
	float GetBattleDashCooldownRatio() const;

	UFUNCTION(BlueprintPure, Category = "Battle|Dash")
	bool CanUseBattleDash() const;

	UPROPERTY(BlueprintAssignable, Category = "Battle|Flow")
	FPBBattleLevelPhaseChangedSignature OnBattleLevelPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Battle|Flow")
	FPBBattleLaunchCountChangedSignature OnBattleLaunchCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Battle|Flow")
	FPBBattleShiftCountChangedSignature OnBattleShiftCountChanged;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Flow")
	EPBBattleLevelPhase CurrentPhase = EPBBattleLevelPhase::DataLoading;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Flow")
	int32 RemainingBattleLaunchCount = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Flow")
	int32 RemainingBattleShiftCount = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Dash")
	float BattleDashCooldownSeconds = 5.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Dash")
	float LastBattleDashUseTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle|Dash")
	bool bBattleDashUsed = false;
	
#pragma region MessageHandler
	
private:
	void RegisterMessageListeners();
	void UnregisterMessageListeners();

	UFUNCTION()
	void HandleBattleLevelPhaseChanged(EPBBattleLevelPhase PreviousPhase, EPBBattleLevelPhase NewPhase);

	UFUNCTION()
	void HandleBattleLaunchCountChanged(int32 PreviousCount, int32 NewCount);

	UFUNCTION()
	void HandleBattleShiftCountChanged(int32 PreviousCount, int32 NewCount);

#pragma endregion 
};
