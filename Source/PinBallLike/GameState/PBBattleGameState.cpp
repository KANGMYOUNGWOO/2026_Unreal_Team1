// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBattleGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossSpawnController.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawnController.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"

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
	InitializeBattleLaunchCount();
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

bool APBBattleGameState::CanLaunchBattleParty() const
{
	return CurrentPhase == EPBBattleLevelPhase::BallDeployment && RemainingBattleLaunchCount > 0;
}

bool APBBattleGameState::NotifyBattlePartyLaunched()
{
	if (!CanLaunchBattleParty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Ignore party launched. CurrentPhase=%s RemainingLaunchCount=%d"),
			*UEnum::GetValueAsString(CurrentPhase),
			RemainingBattleLaunchCount);
		return false;
	}

	--RemainingBattleLaunchCount;
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Party launched. RemainingLaunchCount=%d"),
		RemainingBattleLaunchCount);

	SetBattleLevelPhase(EPBBattleLevelPhase::Combat);
	return true;
}

void APBBattleGameState::NotifyBattlePartyAllBallsDead()
{
	if (CurrentPhase != EPBBattleLevelPhase::Combat)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Party all balls dead. RemainingLaunchCount=%d"),
		RemainingBattleLaunchCount);

	if (RemainingBattleLaunchCount > 0)
	{
		SetBattleLevelPhase(EPBBattleLevelPhase::BallDeployment);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Party all balls dead with no remaining launch count."));
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
	case EPBBattleLevelPhase::BallDeployment:
		HandleBallDeployment();
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

void APBBattleGameState::InitializeBattleLaunchCount()
{
	UGameInstance* GameInstance = GetGameInstance();
	const UPBPlayerDataSubsystem* PlayerDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBPlayerDataSubsystem>() : nullptr;
	RemainingBattleLaunchCount = PlayerDataSubsystem ? PlayerDataSubsystem->GetInitialBattleLaunchCount() : 0;

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Initialize battle launch count. Remaining=%d"),
		RemainingBattleLaunchCount);
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
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Prepare placed balls."));

	UGameInstance* GameInstance = GetGameInstance();
	UPBBallDeckSubsystem* BallDeckSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	if (!BallDeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] PrepareBalls failed. Missing BallDeckSubsystem."));
		MarkPreparationCompleted(EPBBattlePreparationType::Ball, false);
		return;
	}

	BallDeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate::CreateUObject(
		this,
		&APBBattleGameState::HandleBallGameplayAssetsLoaded));
}

void APBBattleGameState::PrepareBoss()
{
	// TODO: 보스 소환 구현 전까지는 준비 완료로 간주한다.
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

void APBBattleGameState::HandleBallGameplayAssetsLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Placed ball gameplay assets loaded."));
	MarkPreparationCompleted(EPBBattlePreparationType::Ball, true);
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

	BossDeadListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattleBossDeadMessage>(
		GameplayTags::Event_Battle_Boss_Dead,
		this,
		&APBBattleGameState::HandleBossDeadMessage);

	PartyLaunchRequestedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyLaunchRequestedMessage>(
		GameplayTags::Event_Battle_Party_Launch_Requested,
		this,
		&APBBattleGameState::HandlePartyLaunchRequestedMessage);

	PartyLaunchedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyLaunchedMessage>(
		GameplayTags::Event_Battle_Party_Launched,
		this,
		&APBBattleGameState::HandlePartyLaunchedMessage);

	PartyAllBallsDeadListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyAllBallsDeadMessage>(
		GameplayTags::Event_Battle_Party_AllBallsDead,
		this,
		&APBBattleGameState::HandlePartyAllBallsDeadMessage);

	PartyShiftRequestedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyShiftRequestedMessage>(
		GameplayTags::Event_Battle_Party_Shift_Requested,
		this,
		&APBBattleGameState::HandlePartyShiftRequestedMessage);

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

	if (BossDeadListenerHandle.IsValid())
	{
		BossDeadListenerHandle.Unregister();
		UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Unregistered boss dead listener."));
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

void APBBattleGameState::HandleBossDeadMessage(
	FGameplayTag Channel,
	const FPBBattleBossDeadMessage& Message)
{
	if (CurrentPhase != EPBBattleLevelPhase::Combat)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Boss dead message. Channel=%s Boss=%s"),
		*Channel.ToString(),
		*GetNameSafe(Message.BossActor));

	SetBattleLevelPhase(EPBBattleLevelPhase::BossDead);
}

void APBBattleGameState::HandlePartyLaunchRequestedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyLaunchRequestedMessage& Message)
{
	if (!CanLaunchBattleParty() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePartyLaunchApprovedMessage ApprovedMessage;
	ApprovedMessage.RemainingLaunchCountBeforeLaunch = RemainingBattleLaunchCount;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Party_Launch_Approved,
		ApprovedMessage);
}

void APBBattleGameState::HandlePartyLaunchedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyLaunchedMessage& Message)
{
	NotifyBattlePartyLaunched();
}

void APBBattleGameState::HandlePartyAllBallsDeadMessage(
	FGameplayTag Channel,
	const FPBBattlePartyAllBallsDeadMessage& Message)
{
	NotifyBattlePartyAllBallsDead();
}

void APBBattleGameState::HandlePartyShiftRequestedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyShiftRequestedMessage& Message)
{
	if (CurrentPhase != EPBBattleLevelPhase::BallDeployment)
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
	SetBattleLevelPhase(EPBBattleLevelPhase::BallDeployment);
}

void APBBattleGameState::HandleBallDeployment_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BallDeployment."));

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBBattlePartyDeploymentStartedMessage Message;
		Message.RemainingLaunchCount = RemainingBattleLaunchCount;
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_Battle_Party_Deployment_Started,
			Message);
	}
}

void APBBattleGameState::HandleBattle_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter Combat."));
}

void APBBattleGameState::HandleBossDead_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter BossDead."));
	SetBattleLevelPhase(EPBBattleLevelPhase::Reward);
}

void APBBattleGameState::HandleReward_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Enter Reward."));
}
