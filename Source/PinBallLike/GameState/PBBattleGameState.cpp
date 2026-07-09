// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBattleGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"

APBBattleGameState::APBBattleGameState()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBBattleGameState::BeginPlay()
{
	Super::BeginPlay();

	RegisterMessageListeners();
}

void APBBattleGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterMessageListeners();

	Super::EndPlay(EndPlayReason);
}

void APBBattleGameState::SetBattleLevelPhase(const EPBBattleLevelPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	const EPBBattleLevelPhase PreviousPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Phase changed. Previous=%s New=%s"),
		*UEnum::GetValueAsString(PreviousPhase),
		*UEnum::GetValueAsString(CurrentPhase));

	OnBattleLevelPhaseChanged.Broadcast(PreviousPhase, CurrentPhase);
}

void APBBattleGameState::SetRemainingBattleLaunchCount(const int32 NewRemainingBattleLaunchCount)
{
	const int32 PreviousBattleLaunchCount = RemainingBattleLaunchCount;
	RemainingBattleLaunchCount = FMath::Max(0, NewRemainingBattleLaunchCount);

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Set remaining battle launch count. Remaining=%d"),
		RemainingBattleLaunchCount);

	if (PreviousBattleLaunchCount != RemainingBattleLaunchCount)
	{
		OnBattleLaunchCountChanged.Broadcast(PreviousBattleLaunchCount, RemainingBattleLaunchCount);
	}
}

bool APBBattleGameState::ConsumeBattleLaunchCount()
{
	if (RemainingBattleLaunchCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Cannot consume battle launch count. Remaining=%d"),
			RemainingBattleLaunchCount);
		return false;
	}

	SetRemainingBattleLaunchCount(RemainingBattleLaunchCount - 1);
	return true;
}

#pragma region MessageHandler

void APBBattleGameState::RegisterMessageListeners()
{
	OnBattleLevelPhaseChanged.AddDynamic(this, &APBBattleGameState::HandleBattleLevelPhaseChanged);
	OnBattleLaunchCountChanged.AddDynamic(this, &APBBattleGameState::HandleBattleLaunchCountChanged);
}

void APBBattleGameState::UnregisterMessageListeners()
{
	OnBattleLevelPhaseChanged.RemoveDynamic(this, &APBBattleGameState::HandleBattleLevelPhaseChanged);
	OnBattleLaunchCountChanged.RemoveDynamic(this, &APBBattleGameState::HandleBattleLaunchCountChanged);
}

void APBBattleGameState::HandleBattleLevelPhaseChanged(
	const EPBBattleLevelPhase PreviousPhase,
	const EPBBattleLevelPhase NewPhase)
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePhaseChangedMessage Message;
	Message.PreviousPhase = PreviousPhase;
	Message.NewPhase = NewPhase;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Phase_Changed,
		Message);
}

void APBBattleGameState::HandleBattleLaunchCountChanged(
	const int32 PreviousCount,
	const int32 NewCount)
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattleLaunchCountChangedMessage Message;
	Message.PreviousCount = PreviousCount;
	Message.NewCount = NewCount;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_LaunchCount_Changed,
		Message);
}

#pragma endregion
