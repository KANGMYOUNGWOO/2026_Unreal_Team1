// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBattleGameMode.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/Boss/PBBossSpawner.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawner.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBEffectSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"

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
	InitializeBattleCounts();
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

void APBBattleGameMode::InitializeBattleCounts()
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
	BattleGameState->SetRemainingBattleShiftCount(
		PlayerDataSubsystem ? PlayerDataSubsystem->GetInitialBattleShiftCount() : 0);
	ApplyActiveSynergyEffectsForBattle();
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
		if (APBBumperSpawner* FoundBumperSpawner = FindBumperSpawner())
		{
			FoundBumperSpawner->LogBattleTelemetrySummary();
		}
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

void APBBattleGameMode::ApplyActiveSynergyEffectsForBattle()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBEffectSubsystem* EffectSubsystem = GameInstance->GetSubsystem<UPBEffectSubsystem>();
	if (!EffectSubsystem)
	{
		return;
	}

	FPBEffectContext EffectContext;
	EffectContext.WorldContextObject = this;
	EffectContext.SourceActor = this;
	EffectSubsystem->NotifyTrigger(GameplayTags::TriggerEvent_Battle_Started, EffectContext);
}

void APBBattleGameMode::TriggerPartySwitchEffects()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APBCombatPartyController> It(World); It; ++It)
	{
		if (APBCombatPartyController* PartyController = *It)
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (UPBEffectSubsystem* EffectSubsystem = GameInstance->GetSubsystem<UPBEffectSubsystem>())
				{
					FPBEffectContext EffectContext;
					EffectContext.WorldContextObject = this;
					EffectContext.SourceActor = PartyController;
					for (AActor* Ball : PartyController->GetValidPartyBalls())
					{
						EffectContext.TargetActors.Add(Ball);
					}
					EffectSubsystem->NotifyTrigger(GameplayTags::TriggerEvent_Battle_PartySwitched, EffectContext);
				}
			}
			return;
		}
	}
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

	APBBumperSpawner* FoundBumperSpawner = FindBumperSpawner();
	if (!IsValid(FoundBumperSpawner))
	{
		MarkDataLoaded(EPBBattlePreparationType::Bumper, false);
		return;
	}

	const FGuid RequestId = FoundBumperSpawner->LoadEquippedBumperDataAssetAsync(
		FStreamableDelegate::CreateUObject(this, &APBBattleGameMode::HandleBumperDataLoaded));
	if (!RequestId.IsValid())
	{
		MarkDataLoaded(EPBBattlePreparationType::Bumper, false);
	}
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

	APBBossSpawner* FoundBossSpawner = FindBossSpawner();
	if (!IsValid(FoundBossSpawner))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Boss data load failed. Missing BossSpawner."));
		MarkDataLoaded(EPBBattlePreparationType::Boss, false);
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UPBPlayerDataSubsystem* PlayerDataSubsystem =
			GameInstance->GetSubsystem<UPBPlayerDataSubsystem>())
		{
			if (const UPBTableDataSubsystem* TableDataSubsystem =
				GameInstance->GetSubsystem<UPBTableDataSubsystem>())
			{
				TArray<FName> BossRowNames;
				if (TableDataSubsystem->GetBossRowNames(BossRowNames)
					&& BossRowNames.IsValidIndex(PlayerDataSubsystem->GetCurrentBossIndex()))
				{
					FoundBossSpawner->SetBossRowName(
						BossRowNames[PlayerDataSubsystem->GetCurrentBossIndex()]);
				}
			}
		}
	}

	const FGuid RequestId = FoundBossSpawner->LoadBossDataAssetAsync(
		FStreamableDelegate::CreateUObject(this, &APBBattleGameMode::HandleBossDataLoaded));
	if (!RequestId.IsValid())
	{
		MarkDataLoaded(EPBBattlePreparationType::Boss, false);
	}
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
	MarkDataLoaded(
		EPBBattlePreparationType::Boss,
		IsValid(BossSpawner) && BossSpawner->IsLoadedBossDataReady());
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

APBBumperSpawner* APBBattleGameMode::FindBumperSpawner()
{
	if (!IsValid(BumperSpawner))
	{
		BumperSpawner = Cast<APBBumperSpawner>(
			UGameplayStatics::GetActorOfClass(this, APBBumperSpawner::StaticClass()));
	}

	return BumperSpawner;
}

APBBossSpawner* APBBattleGameMode::FindBossSpawner()
{
	if (!IsValid(BossSpawner))
	{
		BossSpawner = Cast<APBBossSpawner>(
			UGameplayStatics::GetActorOfClass(this, APBBossSpawner::StaticClass()));
	}

	return BossSpawner;
}

void APBBattleGameMode::PrepareBumpers()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Prepare equipped bumpers."));

	APBBumperSpawner* FoundBumperSpawner = FindBumperSpawner();
	if (!IsValid(FoundBumperSpawner))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing BumperSpawner in level."));
		MarkPreparationCompleted(EPBBattlePreparationType::Bumper, false);
		return;
	}

	FoundBumperSpawner->SpawnLoadedBumpers();
}

void APBBattleGameMode::PrepareBoss()
{
	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Prepare boss."));

	APBBossSpawner* FoundBossSpawner = FindBossSpawner();
	if (!IsValid(FoundBossSpawner))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BattleFlow] Missing BossSpawner in level."));
		MarkPreparationCompleted(EPBBattlePreparationType::Boss, false);
		return;
	}

	FoundBossSpawner->SpawnLoadedBoss();
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

	BossIntroCompletedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattleBossIntroCompletedMessage>(
		GameplayTags::Event_Battle_Boss_Intro_Completed,
		this,
		&APBBattleGameMode::HandleBossIntroCompletedMessage);

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

	if (BossIntroCompletedListenerHandle.IsValid())
	{
		BossIntroCompletedListenerHandle.Unregister();
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

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBPlayerDataSubsystem* PlayerDataSubsystem =
			GameInstance->GetSubsystem<UPBPlayerDataSubsystem>())
		{
			if (const UPBTableDataSubsystem* TableDataSubsystem =
				GameInstance->GetSubsystem<UPBTableDataSubsystem>())
			{
				TArray<FName> BossRowNames;
				if (TableDataSubsystem->GetBossRowNames(BossRowNames))
				{
					PlayerDataSubsystem->AdvanceBossProgress(BossRowNames.Num());
				}
			}
		}
	}

	SetBattleLevelPhase(EPBBattleLevelPhase::BossDead);
}

void APBBattleGameMode::HandleBossIntroCompletedMessage(
	FGameplayTag Channel,
	const FPBBattleBossIntroCompletedMessage& Message)
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	const APBBossBase* SpawnedBoss = BossSpawner ? BossSpawner->GetSpawnedBoss() : nullptr;
	if (!BattleGameState
		|| BattleGameState->GetBattleLevelPhase() != EPBBattleLevelPhase::BossIntro
		|| !SpawnedBoss
		|| Message.BossActor != SpawnedBoss)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BattleFlow] Boss intro completed. Channel=%s Boss=%s"),
		*Channel.ToString(),
		*GetNameSafe(Message.BossActor));

	SetBattleLevelPhase(EPBBattleLevelPhase::BallDeployment);
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
	if (APBBumperSpawner* FoundBumperSpawner = FindBumperSpawner())
	{
		FoundBumperSpawner->LogBattleTelemetrySummary();
	}
}

void APBBattleGameMode::HandlePartyShiftRequestedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyShiftRequestedMessage& Message)
{
	APBBattleGameState* BattleGameState = GetBattleGameState();
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

	if (BallDeckSubsystem->RotateDeploymentSlots())
	{
		TriggerPartySwitchEffects();
	}
}

#pragma endregion
