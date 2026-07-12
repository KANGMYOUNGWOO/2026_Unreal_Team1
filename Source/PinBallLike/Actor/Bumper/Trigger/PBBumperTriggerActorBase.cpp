// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperTriggerActorBase.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"

APBBumperTriggerActorBase::APBBumperTriggerActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBumperTriggerActorBase::InitializeTrigger(
	APBModularBumperBase* InOwnerBumper,
	const EPBBumperPositionId InPositionId,
	const int32 InRequiredTriggerCount)
{
	OwnerBumper = InOwnerBumper;
	PositionId = InPositionId;
	RequiredTriggerCount = FMath::Max(InRequiredTriggerCount, 1);
	CurrentTriggerCount = 0;
	ProgressState = EPBBumperTriggerProgressState::Charging;
}

APBModularBumperBase* APBBumperTriggerActorBase::GetOwnerBumper() const
{
	return OwnerBumper.Get();
}

EPBBumperTriggerType APBBumperTriggerActorBase::GetTriggerType() const
{
	return TriggerType;
}

EPBBumperState APBBumperTriggerActorBase::GetTriggerState() const
{
	return CurrentState;
}

EPBBumperPositionId APBBumperTriggerActorBase::GetPositionId() const
{
	return PositionId;
}

int32 APBBumperTriggerActorBase::GetCurrentTriggerCount() const
{
	return CurrentTriggerCount;
}

int32 APBBumperTriggerActorBase::GetRequiredTriggerCount() const
{
	return RequiredTriggerCount;
}

EPBBumperTriggerProgressState APBBumperTriggerActorBase::GetTriggerProgressState() const
{
	return ProgressState;
}

bool APBBumperTriggerActorBase::IsTriggerReady() const
{
	return ProgressState == EPBBumperTriggerProgressState::Ready
		|| ProgressState == EPBBumperTriggerProgressState::Queued;
}

void APBBumperTriggerActorBase::SetTriggerState(const EPBBumperState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
}

bool APBBumperTriggerActorBase::CanIncreaseTrigger() const
{
	const APBModularBumperBase* Bumper = OwnerBumper.Get();
	return CurrentState == EPBBumperState::Idle
		&& ProgressState == EPBBumperTriggerProgressState::Charging
		&& IsValid(Bumper)
		&& Bumper->CanAccumulateTrigger();
}

bool APBBumperTriggerActorBase::CanReactToBall() const
{
	return CurrentState != EPBBumperState::Disabled;
}

void APBBumperTriggerActorBase::FinishTrigger()
{
	ResetTriggerProgress();
	if (CurrentState != EPBBumperState::Disabled)
	{
		SetTriggerState(EPBBumperState::Idle);
	}
}

void APBBumperTriggerActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (!OwnerBumper.IsValid())
	{
		OwnerBumper = Cast<APBModularBumperBase>(GetOwner());
	}

	NotifyTriggerProgressChanged();
}

void APBBumperTriggerActorBase::IncreaseTrigger(APBBallBase* Ball, const FHitResult& TriggerHit)
{
	if (!IsValid(Ball) || !CanIncreaseTrigger())
	{
		return;
	}

	// 자식 액터가 감지한 판정을 범퍼 본체의 카운트 증가 흐름으로 전달한다.
	OwnerBumper->HandleTriggerActorActivated(this, Ball, TriggerHit);
	OnTriggerActivated(Ball, TriggerHit);
}

bool APBBumperTriggerActorBase::AddTriggerProgress(const int32 Amount)
{
	if (Amount <= 0 || ProgressState != EPBBumperTriggerProgressState::Charging)
	{
		return false;
	}

	const int32 PreviousCount = CurrentTriggerCount;
	CurrentTriggerCount = FMath::Clamp(
		CurrentTriggerCount + Amount,
		0,
		RequiredTriggerCount);
	NotifyTriggerProgressChanged();

	const bool bBecameReady = PreviousCount < RequiredTriggerCount
		&& CurrentTriggerCount >= RequiredTriggerCount;
	if (bBecameReady)
	{
		SetTriggerProgressState(EPBBumperTriggerProgressState::Ready);
	}

	return bBecameReady;
}

void APBBumperTriggerActorBase::ResetTriggerProgress()
{
	CurrentTriggerCount = 0;
	SetTriggerProgressState(EPBBumperTriggerProgressState::Charging);
	NotifyTriggerProgressChanged();
}

void APBBumperTriggerActorBase::SetTriggerProgressState(
	const EPBBumperTriggerProgressState NewState)
{
	if (ProgressState == NewState)
	{
		return;
	}

	const EPBBumperTriggerProgressState PreviousState = ProgressState;
	ProgressState = NewState;
	OnTriggerProgressStateChanged.Broadcast(PreviousState, ProgressState);
}

void APBBumperTriggerActorBase::NotifyTriggerProgressChanged()
{
	OnTriggerProgressChanged.Broadcast(CurrentTriggerCount, RequiredTriggerCount);
}
