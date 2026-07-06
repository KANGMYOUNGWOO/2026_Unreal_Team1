// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBattleGameState.h"

#include "EngineUtils.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawnController.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/GamePlayMessage/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"

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

	if (IsValid(GameDataLoadSubsystem))
	{
		GameDataLoadSubsystem->OnPrimaryAssetsLoaded.RemoveDynamic(
			this,
			&APBBattleGameState::ContinueAfterBumperAssetsLoaded);
	}

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
	// 레벨 준비에 필요한 작업을 시작한다.
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter LevelPreparing."));
	ResetPreparationState();

	PrepareBumpers();
	PrepareBalls();
	PrepareBoss();
}

void APBBattleGameState::PrepareBumpers()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Prepare equipped bumpers."));

	GameDataLoadSubsystem = PBSubsystemUtils::GetGameInstanceSubsystem<UPBGameDataLoadSubsystem>(this);
	if (!IsValid(GameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing GameDataLoadSubsystem."));
		return;
	}

	GameDataLoadSubsystem->OnPrimaryAssetsLoaded.AddUniqueDynamic(
		this,
		&APBBattleGameState::ContinueAfterBumperAssetsLoaded);
	GameDataLoadSubsystem->LoadEquippedBumpersAsync();
}

void APBBattleGameState::ContinueAfterBumperAssetsLoaded()
{
	if (IsValid(GameDataLoadSubsystem))
	{
		GameDataLoadSubsystem->OnPrimaryAssetsLoaded.RemoveDynamic(
			this,
			&APBBattleGameState::ContinueAfterBumperAssetsLoaded);
	}

	if (!IsValid(BumperSpawnController))
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			return;
		}

		for (TActorIterator<APBBumperSpawnController> It(World); It; ++It)
		{
			BumperSpawnController = *It;
			break;
		}
	}

	if (!IsValid(BumperSpawnController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing BumperSpawnController in level."));
		return;
	}

	BumperSpawnController->SpawnEquippedBumpers();
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

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Preparation completed message. Channel=%s Type=%s Requested=%d Completed=%d Success=%s"),
		*Channel.ToString(),
		*UEnum::GetValueAsString(Message.PreparationType),
		Message.RequestedCount,
		Message.CompletedCount,
		Message.bSuccess ? TEXT("true") : TEXT("false"));

	MarkPreparationCompleted(Message.PreparationType, Message.bSuccess);
}

void APBBattleGameState::ResetPreparationState()
{
	bBumperPrepared = false;
	bBallPrepared = false;
	bBossPrepared = false;

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Reset preparation state. Bumper=false Ball=false Boss=false"));
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

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Mark preparation. Type=%s Success=%s CurrentState=(Bumper=%s Ball=%s Boss=%s)"),
		*UEnum::GetValueAsString(PreparationType),
		bSuccess ? TEXT("true") : TEXT("false"),
		bBumperPrepared ? TEXT("true") : TEXT("false"),
		bBallPrepared ? TEXT("true") : TEXT("false"),
		bBossPrepared ? TEXT("true") : TEXT("false"));

	TryAdvanceFromLevelPreparing();
}

void APBBattleGameState::TryAdvanceFromLevelPreparing()
{
	if (CurrentPhase != EPBBattleLevelPhase::LevelPreparing)
	{
		UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Skip preparation advance. CurrentPhase=%s"),
			*UEnum::GetValueAsString(CurrentPhase));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Check preparation advance. Bumper=%s Ball=%s Boss=%s"),
		bBumperPrepared ? TEXT("true") : TEXT("false"),
		bBallPrepared ? TEXT("true") : TEXT("false"),
		bBossPrepared ? TEXT("true") : TEXT("false"));

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
