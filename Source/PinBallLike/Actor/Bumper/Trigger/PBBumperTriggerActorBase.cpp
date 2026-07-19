// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperTriggerActorBase.h"

#include "Components/SceneComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Struct/Battle/PBBumperTriggeredMessage.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

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

bool APBBumperTriggerActorBase::CanReactToMovableActor() const
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

void APBBumperTriggerActorBase::IncreaseTrigger(
	AActor* InteractionActor,
	const FHitResult& TriggerHit)
{
	if (!IsValid(InteractionActor)
		|| !PBInterfaceUtils::FindInterface<IMovable>(InteractionActor)
		|| !CanIncreaseTrigger())
	{
		return;
	}

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBBumperTriggeredMessage Message;
		Message.Bumper = OwnerBumper.Get();
		Message.TriggerLocation = TriggerHit.ImpactPoint;
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_Battle_Bumper_Triggered,
			Message);
	}

	// 자식 액터가 감지한 판정을 범퍼 본체의 카운트 증가 흐름으로 전달한다.
	OwnerBumper->HandleTriggerActorActivated(this, InteractionActor, TriggerHit);
	OnMovableActorTriggered(InteractionActor, TriggerHit);

	// 이미 제작된 Blueprint의 Ball 타입 이벤트는 실제 Ball일 때 계속 호출한다.
	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		OnTriggerActivated(Ball, TriggerHit);
	}
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
