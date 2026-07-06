// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBattleGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawnController.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"

APBBattleGameState::APBBattleGameState()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBBattleGameState::BeginPlay()
{
	Super::BeginPlay();

	RegisterBattleMessageListeners();

	if (bStartFlowOnBeginPlay)
	{
		StartBattleLevelFlow();
	}
}

void APBBattleGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterBattleMessageListeners();

	Super::EndPlay(EndPlayReason);
}

void APBBattleGameState::StartBattleLevelFlow()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Start battle level flow."));
	SetBattleLevelPhase(EPBBattleLevelPhase::LevelPreparing);
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
	HandleCurrentPhase();
}

void APBBattleGameState::CompleteLevelPreparing()
{
	if (CurrentPhase != EPBBattleLevelPhase::LevelPreparing)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Ignore CompleteLevelPreparing. CurrentPhase=%s"),
			*UEnum::GetValueAsString(CurrentPhase));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Level preparing completed. Advance to BossIntro."));
	SetBattleLevelPhase(EPBBattleLevelPhase::BossIntro);
}

void APBBattleGameState::HandleCurrentPhase()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Handle phase. Current=%s"),
		*UEnum::GetValueAsString(CurrentPhase));

	switch (CurrentPhase)
	{
	case EPBBattleLevelPhase::LevelPreparing:
		HandleLevelPreparing();
		break;
	case EPBBattleLevelPhase::BossIntro:
		HandleBossIntro();
		break;
	case EPBBattleLevelPhase::Combat:
		HandleBattle();
		break;
	case EPBBattleLevelPhase::BossDead:
		HandleBossDead();
		break;
	case EPBBattleLevelPhase::Reward:
		HandleReward();
		break;
	default:
		break;
	}
}

void APBBattleGameState::HandleLevelPreparing_Implementation()
{
	// 레벨 준비 단계에서 필요한 작업을 시작한다.
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter LevelPreparing."));
	ResetPreparationState();

	PrepareBumpers();
	PrepareBalls();
	PrepareBoss();
}

void APBBattleGameState::PrepareBumpers()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Prepare equipped bumpers."));

	if (!IsValid(BumperSpawnController))
	{
		BumperSpawnController = Cast<APBBumperSpawnController>(
			UGameplayStatics::GetActorOfClass(this, APBBumperSpawnController::StaticClass()));
	}

	if (!IsValid(BumperSpawnController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing BumperSpawnController in level."));
		MarkPreparationCompleted(EPBBattlePreparationType::Bumper, false);
		return;
	}

	BumperSpawnController->PrepareEquippedBumpersAsync();
}

void APBBattleGameState::PrepareBalls()
{
	// TODO: 볼 소환 구현 전까지는 준비 완료로 간주한다.
	MarkPreparationCompleted(EPBBattlePreparationType::Ball, true);
}

void APBBattleGameState::PrepareBoss()
{
	// TODO: 보스 소환 구현 전까지는 준비 완료로 간주한다.
	MarkPreparationCompleted(EPBBattlePreparationType::Boss, true);
}

void APBBattleGameState::RegisterBattleMessageListeners()
{
	if (!UGameplayMessageSubsystem::HasInstance(this) || PreparationCompletedListenerHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Skip register battle message listener. HasSubsystem=%s HandleValid=%s"),
			UGameplayMessageSubsystem::HasInstance(this) ? TEXT("true") : TEXT("false"),
			PreparationCompletedListenerHandle.IsValid() ? TEXT("true") : TEXT("false"));
		return;
	}

	PreparationCompletedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePreparationCompletedMessage>(
		GameplayTags::Event_Battle_Phase_Prepare_Completed,
		this,
		&APBBattleGameState::HandlePreparationCompletedMessage);

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Registered preparation listener. Channel=%s"),
		*FGameplayTag(GameplayTags::Event_Battle_Phase_Prepare_Completed).ToString());
}

void APBBattleGameState::UnregisterBattleMessageListeners()
{
	if (PreparationCompletedListenerHandle.IsValid())
	{
		PreparationCompletedListenerHandle.Unregister();
		UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Unregistered preparation listener."));
	}
}

void APBBattleGameState::HandlePreparationCompletedMessage(
	FGameplayTag Channel,
	const FPBBattlePreparationCompletedMessage& Message)
{
	if (CurrentPhase != EPBBattleLevelPhase::LevelPreparing)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Preparation completed message. Channel=%s Type=%s Success=%s"),
		*Channel.ToString(),
		*UEnum::GetValueAsString(Message.PreparationType),
		Message.bSuccess ? TEXT("true") : TEXT("false"));

	MarkPreparationCompleted(Message.PreparationType, Message.bSuccess);
}

void APBBattleGameState::ResetPreparationState()
{
	bBumperPrepared = false;
	bBallPrepared = false;
	bBossPrepared = false;
}

void APBBattleGameState::MarkPreparationCompleted(
	const EPBBattlePreparationType PreparationType,
	const bool bSuccess)
{
	switch (PreparationType)
	{
	case EPBBattlePreparationType::Bumper:
		bBumperPrepared = bSuccess;
		break;
	case EPBBattlePreparationType::Ball:
		bBallPrepared = bSuccess;
		break;
	case EPBBattlePreparationType::Boss:
		bBossPrepared = bSuccess;
		break;
	default:
		break;
	}

	if (CurrentPhase != EPBBattleLevelPhase::LevelPreparing)
	{
		UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Skip preparation advance. CurrentPhase=%s"),
			*UEnum::GetValueAsString(CurrentPhase));
		return;
	}
	
	if (bBumperPrepared && bBallPrepared && bBossPrepared)
	{
		CompleteLevelPreparing();
	}
}

void APBBattleGameState::HandleBossIntro_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BossIntro."));
}

void APBBattleGameState::HandleBattle_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter Combat."));
}

void APBBattleGameState::HandleBossDead_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BossDead."));
}

void APBBattleGameState::HandleReward_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter Reward."));
}
