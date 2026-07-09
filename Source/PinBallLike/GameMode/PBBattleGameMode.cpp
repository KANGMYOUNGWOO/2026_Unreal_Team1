// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBattleGameMode.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossSpawnController.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawnController.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"

#pragma region Lifecycle

APBBattleGameMode::APBBattleGameMode()
{
	GameStateClass = APBBattleGameState::StaticClass();
}

void APBBattleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	ResetBattlePreparationState();
	EnterBattleDataLoad();
}

void APBBattleGameMode::StartPlay()
{
	Super::StartPlay();

	RegisterBattleMessageListeners();
	InitializeBattleLaunchCount();
	bStartPlayCompleted = true;
	TryStartLevelPreparing();
}

void APBBattleGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterBattleMessageListeners();

	Super::EndPlay(EndPlayReason);
}

#pragma endregion

#pragma region PhaseFlow

bool APBBattleGameMode::CanLaunchBattleParty() const
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	return BattleGameState
		&& BattleGameState->GetBattleLevelPhase() == EPBBattleLevelPhase::BallDeployment
		&& BattleGameState->HasRemainingBattleLaunchCount();
}

APBBattleGameState* APBBattleGameMode::GetBattleGameState() const
{
	return GetGameState<APBBattleGameState>();
}

void APBBattleGameMode::InitializeBattleLaunchCount()
{
	APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UPBPlayerDataSubsystem* PlayerDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBPlayerDataSubsystem>() : nullptr;
	BattleGameState->SetRemainingBattleLaunchCount(
		PlayerDataSubsystem ? PlayerDataSubsystem->GetInitialBattleLaunchCount() : 0);
}

void APBBattleGameMode::SetBattleLevelPhase(const EPBBattleLevelPhase NewPhase)
{
	APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing BattleGameState."));
		return;
	}

	const EPBBattleLevelPhase PreviousPhase = BattleGameState->GetBattleLevelPhase();
	BattleGameState->SetBattleLevelPhase(NewPhase);

	if (PreviousPhase != NewPhase)
	{
		HandleCurrentPhase();
	}
}

void APBBattleGameMode::HandleCurrentPhase()
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState)
	{
		return;
	}

	const EPBBattleLevelPhase CurrentPhase = BattleGameState->GetBattleLevelPhase();
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Handle phase. Current=%s"),
		*UEnum::GetValueAsString(CurrentPhase));

	switch (CurrentPhase)
	{
	case EPBBattleLevelPhase::DataLoading:
		break;
	case EPBBattleLevelPhase::LevelPreparing:
		EnterLevelPreparing();
		break;
	case EPBBattleLevelPhase::BossIntro:
		EnterBossIntro();
		break;
	case EPBBattleLevelPhase::BallDeployment:
		EnterBallDeployment();
		break;
	case EPBBattleLevelPhase::Combat:
		EnterBattle();
		break;
	case EPBBattleLevelPhase::BossDead:
		EnterBossDead();
		break;
	case EPBBattleLevelPhase::Reward:
		HandleReward();
		break;
	default:
		break;
	}
}

void APBBattleGameMode::EnterBattleDataLoad()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Start battle data load."));

	LoadBumpers();
	LoadBalls();
	LoadBoss();
}

void APBBattleGameMode::TryStartLevelPreparing()
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState
		|| BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::DataLoading
		|| !bStartPlayCompleted
		|| !IsBattleDataLoaded())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Battle data loaded. Advance to LevelPreparing."));
	SetBattleLevelPhase(EPBBattleLevelPhase::LevelPreparing);
}

void APBBattleGameMode::EnterLevelPreparing()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter LevelPreparing."));

	PrepareBumpers();
	PrepareBoss();

	TryStartBossInfo();
}

void APBBattleGameMode::TryStartBossInfo()
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState
		|| BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::LevelPreparing
		|| !IsBattleDataLoaded()
		|| !IsBattlePreparationCompleted())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Battle preparation completed. Advance to BossIntro."));
	SetBattleLevelPhase(EPBBattleLevelPhase::BossIntro);
}

void APBBattleGameMode::EnterBossIntro()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BossIntro."));
	SetBattleLevelPhase(EPBBattleLevelPhase::BallDeployment);
}

void APBBattleGameMode::EnterBallDeployment()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BallDeployment."));
}

void APBBattleGameMode::EnterBattle()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter Combat."));
}

void APBBattleGameMode::EnterBossDead()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BossDead."));
	SetBattleLevelPhase(EPBBattleLevelPhase::Reward);
}

void APBBattleGameMode::HandleReward_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter Reward."));
}

#pragma endregion

#pragma region DataLoad

void APBBattleGameMode::LoadBumpers()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Load bumper data."));

	// TODO 범퍼 데이터 로드 분할 필요
	HandleBumperDataLoaded();
}

void APBBattleGameMode::LoadBalls()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Load ball data."));

	UGameInstance* GameInstance = GetGameInstance();
	UPBBallDeckSubsystem* BallDeckSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	if (!BallDeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Ball data load failed. Missing BallDeckSubsystem."));
		MarkDataLoaded(EPBBattlePreparationType::Ball, false);
		return;
	}

	BallDeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate::CreateUObject(
		this,
		&APBBattleGameMode::HandleBallDataLoaded));
}

void APBBattleGameMode::LoadBoss()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Load boss data."));

	// TODO 보스 데이터 로드 분할 필요
	HandleBossDataLoaded();
}

void APBBattleGameMode::HandleBumperDataLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Bumper data loaded."));
	MarkDataLoaded(EPBBattlePreparationType::Bumper, true);
}

void APBBattleGameMode::HandleBallDataLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Ball data loaded."));
	MarkDataLoaded(EPBBattlePreparationType::Ball, true);
}

void APBBattleGameMode::HandleBossDataLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Boss data loaded."));
	MarkDataLoaded(EPBBattlePreparationType::Boss, true);
}

void APBBattleGameMode::MarkDataLoaded(
	const EPBBattlePreparationType PreparationType,
	const bool bSuccess)
{
	switch (PreparationType)
	{
	case EPBBattlePreparationType::Bumper:
		bBumperDataLoaded = bSuccess;
		break;
	case EPBBattlePreparationType::Ball:
		bBallDataLoaded = bSuccess;
		break;
	case EPBBattlePreparationType::Boss:
		bBossDataLoaded = bSuccess;
		break;
	default:
		break;
	}
	
	TryStartLevelPreparing();
}

bool APBBattleGameMode::IsBattleDataLoaded() const
{
	return bBumperDataLoaded && bBallDataLoaded && bBossDataLoaded;
}

#pragma endregion

#pragma region LevelPreparing

void APBBattleGameMode::ResetBattlePreparationState()
{
	bBumperDataLoaded = false;
	bBallDataLoaded = false;
	bBossDataLoaded = false;
	bBumperPrepared = false;
	bBossPrepared = false;
	bStartPlayCompleted = false;
}

void APBBattleGameMode::PrepareBumpers()
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

void APBBattleGameMode::PrepareBoss()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Prepare boss."));

	if (!IsValid(BossSpawnController))
	{
		BossSpawnController = Cast<APBBossSpawnController>(
			UGameplayStatics::GetActorOfClass(this, APBBossSpawnController::StaticClass()));
	}

	if (!IsValid(BossSpawnController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing BossSpawnController in level."));
		MarkPreparationCompleted(EPBBattlePreparationType::Boss, false);
		return;
	}

	BossSpawnController->SpawnBossAsync();
}

void APBBattleGameMode::MarkPreparationCompleted(
	const EPBBattlePreparationType PreparationType,
	const bool bSuccess)
{
	switch (PreparationType)
	{
	case EPBBattlePreparationType::Bumper:
		bBumperPrepared = bSuccess;
		break;
	case EPBBattlePreparationType::Boss:
		bBossPrepared = bSuccess;
		break;
	default:
		break;
	}

	TryStartBossInfo();
}

bool APBBattleGameMode::IsBattlePreparationCompleted() const
{
	return bBumperPrepared && bBossPrepared;
}

#pragma endregion

#pragma region MessageHandler

void APBBattleGameMode::RegisterBattleMessageListeners()
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
		&APBBattleGameMode::HandlePreparationCompletedMessage);

	BossDeadListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattleBossDeadMessage>(
		GameplayTags::Event_Battle_Boss_Dead,
		this,
		&APBBattleGameMode::HandleBossDeadMessage);

	PartyLaunchRequestedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyLaunchRequestedMessage>(
		GameplayTags::Event_Battle_Party_Launch_Requested,
		this,
		&APBBattleGameMode::HandlePartyLaunchRequestedMessage);

	PartyLaunchedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyLaunchedMessage>(
		GameplayTags::Event_Battle_Party_Launched,
		this,
		&APBBattleGameMode::HandlePartyLaunchedMessage);

	PartyAllBallsDeadListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyAllBallsDeadMessage>(
		GameplayTags::Event_Battle_Party_AllBallsDead,
		this,
		&APBBattleGameMode::HandlePartyAllBallsDeadMessage);

	PartyShiftRequestedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyShiftRequestedMessage>(
		GameplayTags::Event_Battle_Party_Shift_Requested,
		this,
		&APBBattleGameMode::HandlePartyShiftRequestedMessage);
}

void APBBattleGameMode::UnregisterBattleMessageListeners()
{
	if (PreparationCompletedListenerHandle.IsValid())
	{
		PreparationCompletedListenerHandle.Unregister();
	}

	if (BossDeadListenerHandle.IsValid())
	{
		BossDeadListenerHandle.Unregister();
	}

	if (PartyLaunchRequestedListenerHandle.IsValid())
	{
		PartyLaunchRequestedListenerHandle.Unregister();
	}

	if (PartyLaunchedListenerHandle.IsValid())
	{
		PartyLaunchedListenerHandle.Unregister();
	}

	if (PartyAllBallsDeadListenerHandle.IsValid())
	{
		PartyAllBallsDeadListenerHandle.Unregister();
	}

	if (PartyShiftRequestedListenerHandle.IsValid())
	{
		PartyShiftRequestedListenerHandle.Unregister();
	}
}

void APBBattleGameMode::HandlePreparationCompletedMessage(
	FGameplayTag Channel,
	const FPBBattlePreparationCompletedMessage& Message)
{
	APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState || BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::LevelPreparing)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Preparation completed message. Channel=%s Type=%s Success=%s"),
		*Channel.ToString(),
		*UEnum::GetValueAsString(Message.PreparationType),
		Message.bSuccess ? TEXT("true") : TEXT("false"));

	MarkPreparationCompleted(Message.PreparationType, Message.bSuccess);
}

void APBBattleGameMode::HandleBossDeadMessage(
	FGameplayTag Channel,
	const FPBBattleBossDeadMessage& Message)
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState || BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::Combat)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Boss dead message. Channel=%s Boss=%s"),
		*Channel.ToString(),
		*GetNameSafe(Message.BossActor));

	SetBattleLevelPhase(EPBBattleLevelPhase::BossDead);
}

void APBBattleGameMode::HandlePartyLaunchRequestedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyLaunchRequestedMessage& Message)
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState || !CanLaunchBattleParty() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePartyLaunchApprovedMessage ApprovedMessage;
	ApprovedMessage.RemainingLaunchCountBeforeLaunch = BattleGameState->GetRemainingBattleLaunchCount();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Party_Launch_Approved,
		ApprovedMessage);
}

void APBBattleGameMode::HandlePartyLaunchedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyLaunchedMessage& Message)
{
	APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState || !CanLaunchBattleParty())
	{
		return;
	}

	if (BattleGameState->ConsumeBattleLaunchCount())
	{
		SetBattleLevelPhase(EPBBattleLevelPhase::Combat);
	}
}

void APBBattleGameMode::HandlePartyAllBallsDeadMessage(
	FGameplayTag Channel,
	const FPBBattlePartyAllBallsDeadMessage& Message)
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState || BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::Combat)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Party all balls dead. RemainingLaunchCount=%d"),
		BattleGameState->GetRemainingBattleLaunchCount());

	if (BattleGameState->HasRemainingBattleLaunchCount())
	{
		SetBattleLevelPhase(EPBBattleLevelPhase::BallDeployment);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Party all balls dead with no remaining launch count."));
}

void APBBattleGameMode::HandlePartyShiftRequestedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyShiftRequestedMessage& Message)
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	if (!BattleGameState || BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::BallDeployment)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPBBallDeckSubsystem* BallDeckSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	if (!BallDeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Ignore party shift request. Missing BallDeckSubsystem."));
		return;
	}

	BallDeckSubsystem->RotateDeploymentSlots();
}

#pragma endregion
