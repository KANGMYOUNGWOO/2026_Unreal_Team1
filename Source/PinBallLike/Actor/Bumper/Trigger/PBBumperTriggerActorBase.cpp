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

void APBBumperTriggerActorBase::InitializeTrigger(APBModularBumperBase* InOwnerBumper)
{
	OwnerBumper = InOwnerBumper;
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
		&& IsValid(Bumper)
		&& Bumper->CanAccumulateTrigger();
}

bool APBBumperTriggerActorBase::CanReactToBall() const
{
	return CurrentState != EPBBumperState::Disabled;
}

void APBBumperTriggerActorBase::FinishTrigger()
{
	if (CurrentState != EPBBumperState::Activated)
	{
		return;
	}

	SetTriggerState(EPBBumperState::Idle);
}

void APBBumperTriggerActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (!OwnerBumper.IsValid())
	{
		OwnerBumper = Cast<APBModularBumperBase>(GetOwner());
	}
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
