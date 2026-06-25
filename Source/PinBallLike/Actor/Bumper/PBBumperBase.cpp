// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperBase.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBBumperBase::APBBumperBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBBumperBase::IncreaseTriggerCount(AActor* OtherActor, int32 Amount)
{
	if (IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(OtherActor))
	{
		Comboable->AddCombo(Amount);
		//UE_LOG(LogTemp, Warning, TEXT("combo = %d"), Comboable->GetCombo());
	}
}

void APBBumperBase::AddTriggerCount(APBBallBase* Ball, const int32 Amount)
{
	if (!IsValid(Ball) || !CanAccumulateTrigger() || Amount <= 0)
	{
		return;
	}

	const int32 RequiredTriggerCount = FMath::Max(BumperData.RequiredTriggerCount, 1);
	const int32 PreviousTriggerCount = CurrentTriggerCount;
	CurrentTriggerCount = FMath::Clamp(
		CurrentTriggerCount + Amount,0,RequiredTriggerCount);
	OnTriggerCountChanged(CurrentTriggerCount, RequiredTriggerCount);

	// 이번 누적으로 처음 조건을 만족했을 때만 Ready 이벤트를 보낸다.
	const bool IsTriggerCountReached = CurrentTriggerCount >= RequiredTriggerCount;
	if (PreviousTriggerCount < RequiredTriggerCount && IsTriggerCountReached)
	{
		OnBumperReady();
	}

	if (IsTriggerCountReached)
	{
		ActivateBumper(Ball);
	}
}

void APBBumperBase::ActivateBumper(APBBallBase* Ball)
{
	if (IsActivating)
	{
		return;
	}
	
	IsActivating = true;
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

void APBBumperBase::ApplyBumperEffect_Implementation(APBBallBase* Ball)
{
	// 자식에서 범퍼 활성 효과를 구현하지 않은 경우 즉시 종료한다.
	FinishActivation();
}
