// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperBase.h"

#include "PinBallLike/Actor/Ball/BallBase.h"

APBBumperBase::APBBumperBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBBumperBase::AddTriggerCount(ABallBase* Ball, const int32 Amount)
{
	if (!IsValid(Ball) || !CanAccumulateTrigger() || Amount <= 0)
	{
		return;
	}

	const int32 RequiredTriggerCount = FMath::Max(BumperData.RequiredTriggerCount, 1);
	CurrentTriggerCount = FMath::Clamp(
		CurrentTriggerCount + Amount,
		0,
		RequiredTriggerCount);
	OnTriggerCountChanged(CurrentTriggerCount, RequiredTriggerCount);

	const bool IsAlreadyReady = IsReady;
	IsReady = CurrentTriggerCount >= RequiredTriggerCount;
	if (IsReady && !IsAlreadyReady)
	{
		OnBumperReady();
	}

	if (CanActivate())
	{
		ActivateBumper(Ball);
	}
}

void APBBumperBase::ActivateBumper(ABallBase* Ball)
{
	if (!CanActivate())
	{
		return;
	}

	IsActivating = true;
	IsReady = false;
	ResetTriggerCount();

	OnBumperActivated(Ball);
	ApplyBumperEffect(Ball);
}

void APBBumperBase::FinishActivation()
{
	if (!IsActivating)
	{
		return;
	}

	IsActivating = false;
	OnBumperFinished();
}

void APBBumperBase::ResetTriggerCount()
{
	CurrentTriggerCount = 0;
	IsReady = false;
	OnTriggerCountChanged(CurrentTriggerCount, FMath::Max(BumperData.RequiredTriggerCount, 1));
}

void APBBumperBase::SetIsEnabled(const bool IsNewEnabled)
{
	IsEnabled = IsNewEnabled;
	if (!IsEnabled)
	{
		ResetTriggerCount();
		IsActivating = false;
	}
}

bool APBBumperBase::CanActivate() const
{
	return IsEnabled
		&& !IsActivating
		&& CurrentTriggerCount >= FMath::Max(BumperData.RequiredTriggerCount, 1);
}

bool APBBumperBase::CanAccumulateTrigger() const
{
	return IsEnabled && !IsActivating;
}

void APBBumperBase::ApplyBumperEffect_Implementation(ABallBase* Ball)
{
	FinishActivation();
}
