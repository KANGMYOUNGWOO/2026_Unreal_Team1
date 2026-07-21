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

void APBBattleGameState::SetRemainingBattleShiftCount(const int32 NewRemainingBattleShiftCount)
{
	const int32 PreviousBattleShiftCount = RemainingBattleShiftCount;
	RemainingBattleShiftCount = FMath::Max(0, NewRemainingBattleShiftCount);

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Set remaining battle shift count. Remaining=%d"),
		RemainingBattleShiftCount);

	if (PreviousBattleShiftCount != RemainingBattleShiftCount)
	{
		OnBattleShiftCountChanged.Broadcast(PreviousBattleShiftCount, RemainingBattleShiftCount);
	}
}

bool APBBattleGameState::ConsumeBattleShiftCount()
{
	if (RemainingBattleShiftCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Cannot consume battle shift count. Remaining=%d"),
			RemainingBattleShiftCount);
		return false;
	}

	SetRemainingBattleShiftCount(RemainingBattleShiftCount - 1);
	return true;
}

void APBBattleGameState::SetBattleDashCooldownSeconds(const float NewBattleDashCooldownSeconds)
{
	BattleDashCooldownSeconds = FMath::Max(0.0f, NewBattleDashCooldownSeconds);
}

bool APBBattleGameState::ConsumeBattleDash()
{
	if (!CanUseBattleDash())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BattleFlow] Cannot consume battle dash. RemainingCooldown=%.2f"),
			GetRemainingBattleDashCooldown());
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	LastBattleDashUseTimeSeconds = World->GetTimeSeconds();
	bBattleDashUsed = true;
	return true;
}

float APBBattleGameState::GetRemainingBattleDashCooldown() const
{
	if (!bBattleDashUsed)
	{
		return 0.0f;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return BattleDashCooldownSeconds;
	}

	const float ElapsedSeconds = World->GetTimeSeconds() - LastBattleDashUseTimeSeconds;
	return FMath::Max(BattleDashCooldownSeconds - ElapsedSeconds, 0.0f);
}

bool APBBattleGameState::CanUseBattleDash() const
{
	return GetRemainingBattleDashCooldown() <= 0.0f;
}

#pragma region MessageHandler

void APBBattleGameState::RegisterMessageListeners()
{
	OnBattleLevelPhaseChanged.AddDynamic(this, &APBBattleGameState::HandleBattleLevelPhaseChanged);
	OnBattleLaunchCountChanged.AddDynamic(this, &APBBattleGameState::HandleBattleLaunchCountChanged);
	OnBattleShiftCountChanged.AddDynamic(this, &APBBattleGameState::HandleBattleShiftCountChanged);
}

void APBBattleGameState::UnregisterMessageListeners()
{
	OnBattleLevelPhaseChanged.RemoveDynamic(this, &APBBattleGameState::HandleBattleLevelPhaseChanged);
	OnBattleLaunchCountChanged.RemoveDynamic(this, &APBBattleGameState::HandleBattleLaunchCountChanged);
	OnBattleShiftCountChanged.RemoveDynamic(this, &APBBattleGameState::HandleBattleShiftCountChanged);
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

void APBBattleGameState::HandleBattleShiftCountChanged(
	const int32 PreviousCount,
	const int32 NewCount)
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattleShiftCountChangedMessage Message;
	Message.PreviousCount = PreviousCount;
	Message.NewCount = NewCount;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_ShiftCount_Changed,
		Message);
}

#pragma endregion
