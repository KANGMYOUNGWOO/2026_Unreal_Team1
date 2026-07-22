#include "PBBossPatternComponent.h"

#include "Engine/World.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"

UPBBossPatternComponent::UPBBossPatternComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossPatternComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APBBossBase>(GetOwner());
	InitializePatterns();
	RegisterBattlePhaseListener();

	const APBBattleGameState* BattleGameState = GetWorld() ? GetWorld()->GetGameState<APBBattleGameState>() : nullptr;
	if (!BattleGameState || BattleGameState->GetBattleLevelPhase() == EPBBattleLevelPhase::Combat)
	{
		IsCombatPhaseActive = true;
	}
}

void UPBBossPatternComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterBattlePhaseListener();
	StopPatternSystem();
	Super::EndPlay(EndPlayReason);
}

void UPBBossPatternComponent::StartPatternSystem()
{
	if (IsPatternSystemActive)
	{
		return;
	}

	IsPatternSystemPaused = false;
	IsPatternSystemActive = true;
	ScheduleNextPatternCheck();
}

void UPBBossPatternComponent::StopPatternSystem()
{
	IsPatternSystemPaused = false;
	PatternSystemPausedTime = 0.0f;
	DeactivatePatternSystem(false);
}

bool UPBBossPatternComponent::PausePatternSystem()
{
	if (!IsPatternSystemActive)
	{
		return false;
	}

	IsPatternSystemPaused = true;
	PatternSystemPausedTime = GetCurrentTimeSeconds();

	if (CurrentPattern && CurrentPattern->PausePatternForExternalGroggy(OwnerBoss))
	{
		IsPatternSystemActive = false;
		ClearPatternCheckTimer();
		return true;
	}

	DeactivatePatternSystem(true);

	return true;
}

bool UPBBossPatternComponent::ResumePatternSystem()
{
	if (!IsPatternSystemPaused)
	{
		return false;
	}

	const float PausedDuration = FMath::Max(0.0f, GetCurrentTimeSeconds() - PatternSystemPausedTime);
	ShiftPatternTimers(PausedDuration);
	PatternSystemPausedTime = 0.0f;

	if (CurrentPattern && IsPatternRunning)
	{
		IsPatternSystemPaused = false;
		IsPatternSystemActive = true;
		if (OwnerBoss)
		{
			OwnerBoss->RequestBossState(EPBBossState::Pattern);
		}

		if (CurrentPattern->ResumePatternAfterExternalGroggy(OwnerBoss))
		{
			return true;
		}
	}

	StartPatternSystem();

	return true;
}

void UPBBossPatternComponent::TryStartNextPattern()
{
	ClearPatternCheckTimer();

	if (!CanStartPattern())
	{
		ScheduleNextPatternCheck();
		return;
	}

	UPBBossPatternBase* NextPattern = SelectExecutablePattern();
	if (!NextPattern)
	{
		ScheduleNextPatternCheck();
		return;
	}

	CurrentPattern = NextPattern;
	IsPatternRunning = true;
	OwnerBoss->RequestBossState(EPBBossState::Pattern);
	if (IsEnragedEntryPattern(CurrentPattern))
	{
		IsEnragedEntryPatternPending = false;
	}
	OnPatternStarted.Broadcast(CurrentPattern);

	CurrentPattern->StartPattern(OwnerBoss);
}

void UPBBossPatternComponent::CancelCurrentPattern()
{
	CancelCurrentPatternInternal(false);
}

void UPBBossPatternComponent::CancelCurrentPatternInternal(bool IsApplyCooldown)
{
	if (!CurrentPattern)
	{
		IsPatternRunning = false;
		return;
	}

	UPBBossPatternBase* CancelledPattern = CurrentPattern;

	if (IsApplyCooldown)
	{
		SetPatternCooldown(CancelledPattern);
	}

	ClearCurrentPattern();

	CancelledPattern->CancelPattern(OwnerBoss);
	SetOwnerBossIdleIfPatternState();

	OnPatternCancelled.Broadcast(CancelledPattern);
}

void UPBBossPatternComponent::NotifyPatternFinished(UPBBossPatternBase* FinishedPattern)
{
	if (!FinishedPattern || CurrentPattern != FinishedPattern)
	{
		return;
	}

	SetPatternCooldown(FinishedPattern);

	ClearCurrentPattern();
	SetOwnerBossIdleIfPatternState();

	OnPatternFinished.Broadcast(FinishedPattern);

	if (IsPatternSystemActive)
	{
		ScheduleNextPatternCheck();
	}
}

void UPBBossPatternComponent::NotifyEnragedPhaseStarted()
{
	CancelCurrentPatternInternal(false);
	ResetPatternCooldowns();
	IsEnragedEntryPatternPending = EnragedEntryPatternInstances.Num() > 0;
}

void UPBBossPatternComponent::ResetPatternCooldowns()
{
	CooldownEndTimeMap.Reset();

	if (OwnerBoss && OwnerBoss->IsEnragedPhase())
	{
		for (UPBBossPatternBase* Pattern : EnragedEntryPatternInstances)
		{
			SetPatternCooldown(Pattern);
		}

		for (UPBBossPatternBase* Pattern : EnragedPatternInstances)
		{
			SetPatternCooldown(Pattern);
		}
	}

	if (IsPatternSystemActive && !IsPatternRunning)
	{
		ClearPatternCheckTimer();
		ScheduleNextPatternCheck();
	}
}

bool UPBBossPatternComponent::CanStartPattern() const
{
	return IsPatternSystemActive
		&& !IsPatternRunning
		&& !CurrentPattern
		&& OwnerBoss
		&& !OwnerBoss->IsDead()
		&& OwnerBoss->GetBossState() != EPBBossState::Groggy
		&& OwnerBoss->GetBossState() != EPBBossState::Dead
		&& IsCombatPhaseActive;
}

UPBBossPatternBase* UPBBossPatternComponent::GetCurrentPattern() const
{
	return CurrentPattern;
}

void UPBBossPatternComponent::ConfigurePatternData(
	const TArray<FPBBossPatternData>& NewPatternDatas,
	const TArray<FPBBossPatternData>& NewEnragedPatternDatas,
	const TArray<FPBBossPatternData>& NewEnragedEntryPatternDatas,
	float NewPatternCheckIntervalSeconds)
{
	PatternDatas = NewPatternDatas;
	EnragedPatternDatas = NewEnragedPatternDatas;
	EnragedEntryPatternDatas = NewEnragedEntryPatternDatas;
	PatternCheckIntervalSeconds = FMath::Max(0.1f, NewPatternCheckIntervalSeconds);
}

void UPBBossPatternComponent::InitializePatterns()
{
	PatternInstances.Reset();
	EnragedPatternInstances.Reset();
	EnragedEntryPatternInstances.Reset();
	CooldownEndTimeMap.Reset();

	InitializePatternDatas(PatternDatas, PatternInstances);
	InitializePatternDatas(EnragedPatternDatas, EnragedPatternInstances);
	InitializePatternDatas(EnragedEntryPatternDatas, EnragedEntryPatternInstances);
}

void UPBBossPatternComponent::RegisterBattlePhaseListener()
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	BattlePhaseChangedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePhaseChangedMessage>(
		GameplayTags::Event_Battle_Phase_Changed,
		this,
		&UPBBossPatternComponent::HandleBattlePhaseChangedMessage);
}

void UPBBossPatternComponent::UnregisterBattlePhaseListener()
{
	if (!BattlePhaseChangedListenerHandle.IsValid())
	{
		return;
	}

	BattlePhaseChangedListenerHandle.Unregister();
	BattlePhaseChangedListenerHandle = FGameplayMessageListenerHandle();
}

void UPBBossPatternComponent::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	static_cast<void>(Channel);

	IsCombatPhaseActive = Message.NewPhase == EPBBattleLevelPhase::Combat;
	if (!IsCombatPhaseActive)
	{
		StopPatternSystem();
		return;
	}

	StartPatternSystem();
}

void UPBBossPatternComponent::InitializePatternDatas(
	const TArray<FPBBossPatternData>& PatternDataList,
	TArray<TObjectPtr<UPBBossPatternBase>>& PatternInstanceList)
{
	for (const FPBBossPatternData& PatternData : PatternDataList)
	{
		if (!PatternData.IsEnabled || PatternData.PatternClass.IsNull())
		{
			continue;
		}

		TSubclassOf<UPBBossPatternBase> PatternClass = PatternData.PatternClass.Get();
		if (!PatternClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BossPatternComponent] InitializePatternDatas failed. Pattern class load failed. Boss=%s PatternName=%s Path=%s"),
				*GetNameSafe(OwnerBoss),
				*PatternData.PatternName.ToString(),
				*PatternData.PatternClass.ToSoftObjectPath().ToString());
			continue;
		}

		UPBBossPatternBase* Pattern = NewObject<UPBBossPatternBase>(this, PatternClass);
		if (!Pattern)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BossPatternComponent] InitializePatternDatas failed. PatternName=%s PatternClass=%s"),
				*PatternData.PatternName.ToString(),
				*GetNameSafe(PatternClass));
			continue;
		}

		Pattern->PatternName = PatternData.PatternName;
		Pattern->CooldownSeconds = FMath::Max(0.0f, PatternData.CooldownSeconds);
		Pattern->DamageAmount = FMath::Max(0, PatternData.DamageAmount);
		Pattern->IsEnabled = PatternData.IsEnabled;
		Pattern->InitializePattern(this);
		PatternInstanceList.Add(Pattern);
	}
}

void UPBBossPatternComponent::ScheduleNextPatternCheck()
{
	if (!IsPatternSystemActive || !GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		PatternCheckTimerHandle,
		this,
		&UPBBossPatternComponent::TryStartNextPattern,
		PatternCheckIntervalSeconds,
		false);
}

void UPBBossPatternComponent::ClearPatternCheckTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PatternCheckTimerHandle);
	}
}

void UPBBossPatternComponent::DeactivatePatternSystem(bool IsApplyCurrentPatternCooldown)
{
	IsPatternSystemActive = false;
	CancelCurrentPatternInternal(IsApplyCurrentPatternCooldown);
	ClearPatternCheckTimer();
}

void UPBBossPatternComponent::SetPatternCooldown(UPBBossPatternBase* Pattern)
{
	if (!Pattern || !GetWorld())
	{
		return;
	}

	CooldownEndTimeMap.FindOrAdd(Pattern) = GetCurrentTimeSeconds() + Pattern->CooldownSeconds;
}

void UPBBossPatternComponent::ClearCurrentPattern()
{
	CurrentPattern = nullptr;
	IsPatternRunning = false;
}

void UPBBossPatternComponent::SetOwnerBossIdleIfPatternState() const
{
	if (OwnerBoss
		&& !OwnerBoss->IsDead()
		&& OwnerBoss->GetBossState() == EPBBossState::Pattern)
	{
		OwnerBoss->RequestBossState(EPBBossState::Idle);
	}
}

void UPBBossPatternComponent::ShiftPatternTimers(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f)
	{
		return;
	}

	for (TPair<const UPBBossPatternBase*, float>& CooldownEndTimePair : CooldownEndTimeMap)
	{
		if (CooldownEndTimePair.Value <= PatternSystemPausedTime)
		{
			continue;
		}

		CooldownEndTimePair.Value += DeltaSeconds;
	}
}

float UPBBossPatternComponent::GetCurrentTimeSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}

float UPBBossPatternComponent::GetPatternCooldownEndTime(const UPBBossPatternBase* Pattern) const
{
	if (!Pattern)
	{
		return 0.0f;
	}

	const float* CooldownEndTime = CooldownEndTimeMap.Find(Pattern);
	return CooldownEndTime ? *CooldownEndTime : 0.0f;
}

bool UPBBossPatternComponent::IsPatternCooldownReady(const UPBBossPatternBase* Pattern) const
{
	if (!Pattern)
	{
		return false;
	}

	return GetCurrentTimeSeconds() >= GetPatternCooldownEndTime(Pattern);
}

UPBBossPatternBase* UPBBossPatternComponent::SelectExecutablePattern() const
{
	if (IsEnragedEntryPatternPending)
	{
		if (UPBBossPatternBase* EnragedEntryPattern = SelectExecutablePatternFromList(EnragedEntryPatternInstances))
		{
			return EnragedEntryPattern;
		}
	}

	return SelectExecutablePatternFromList(GetCurrentPhasePatternInstances());
}

UPBBossPatternBase* UPBBossPatternComponent::SelectExecutablePatternFromList(const TArray<TObjectPtr<UPBBossPatternBase>>& PatternInstanceList) const
{
	UPBBossPatternBase* SelectedPattern = nullptr;
	float SelectedCooldownEndTime = TNumericLimits<float>::Max();

	for (UPBBossPatternBase* Pattern : PatternInstanceList)
	{
		if (!Pattern)
		{
			continue;
		}

		const bool IsCooldownReady = IsPatternCooldownReady(Pattern);
		const bool IsCanExecute = IsCooldownReady && Pattern->CanExecute(OwnerBoss);

		if (!IsCanExecute)
		{
			continue;
		}

		const float PatternCooldownEndTime = GetPatternCooldownEndTime(Pattern);

		if (!SelectedPattern || PatternCooldownEndTime < SelectedCooldownEndTime)
		{
			SelectedPattern = Pattern;
			SelectedCooldownEndTime = PatternCooldownEndTime;
		}
	}

	return SelectedPattern;
}

const TArray<TObjectPtr<UPBBossPatternBase>>& UPBBossPatternComponent::GetCurrentPhasePatternInstances() const
{
	if (OwnerBoss && OwnerBoss->IsEnragedPhase())
	{
		return EnragedPatternInstances;
	}

	return PatternInstances;
}

bool UPBBossPatternComponent::IsEnragedEntryPattern(const UPBBossPatternBase* Pattern) const
{
	return Pattern && EnragedEntryPatternInstances.Contains(Pattern);
}
