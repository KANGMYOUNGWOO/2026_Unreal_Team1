#include "PBBossPatternComponent.h"

#include "PBBossPatternBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"

UPBBossPatternComponent::UPBBossPatternComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossPatternComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APBBossBase>(GetOwner());
	InitializePatterns();
}

void UPBBossPatternComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPatternSystem();
	Super::EndPlay(EndPlayReason);
}

void UPBBossPatternComponent::StartPatternSystem()
{
	if (IsPatternSystemActive)
	{
		return;
	}

	IsPatternSystemActive = true;
	ScheduleNextPatternCheck();
}

void UPBBossPatternComponent::StopPatternSystem()
{
	IsPatternSystemActive = false;
	CancelCurrentPattern();
	ClearPatternCheckTimer();
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
	OwnerBoss->SetBossState(EPBBossState::Pattern);
	OnPatternStarted.Broadcast(CurrentPattern);

	CurrentPattern->StartPattern(OwnerBoss);
}

void UPBBossPatternComponent::CancelCurrentPattern()
{
	if (!CurrentPattern)
	{
		IsPatternRunning = false;
		return;
	}

	UPBBossPatternBase* CancelledPattern = CurrentPattern;
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
	NextPatternAllowedTime = GetCurrentTimeSeconds() + MinPatternIntervalSeconds;

	ClearCurrentPattern();
	SetOwnerBossIdleIfPatternState();

	OnPatternFinished.Broadcast(FinishedPattern);

	if (IsPatternSystemActive)
	{
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
		&& GetCurrentTimeSeconds() >= NextPatternAllowedTime;
}

UPBBossPatternBase* UPBBossPatternComponent::GetCurrentPattern() const
{
	return CurrentPattern;
}

void UPBBossPatternComponent::InitializePatterns()
{
	PatternInstances.Reset();
	CooldownEndTimeMap.Reset();

	for (TSubclassOf<UPBBossPatternBase> PatternClass : PatternClasses)
	{
		if (!PatternClass)
		{
			continue;
		}

		UPBBossPatternBase* Pattern = NewObject<UPBBossPatternBase>(this, PatternClass);
		if (!Pattern)
		{
			continue;
		}

		Pattern->InitializePattern(this);
		PatternInstances.Add(Pattern);
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
	if (OwnerBoss && OwnerBoss->GetBossState() == EPBBossState::Pattern)
	{
		OwnerBoss->SetBossState(EPBBossState::Idle);
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
	UPBBossPatternBase* SelectedPattern = nullptr;
	float SelectedCooldownEndTime = TNumericLimits<float>::Max();

	for (UPBBossPatternBase* Pattern : PatternInstances)
	{
		if (!Pattern || !IsPatternCooldownReady(Pattern) || !Pattern->CanExecute(OwnerBoss))
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
