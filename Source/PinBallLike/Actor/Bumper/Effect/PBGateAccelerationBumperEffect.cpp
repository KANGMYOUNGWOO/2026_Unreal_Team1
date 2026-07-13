// Fill out your copyright notice in the Description page of Project Settings.

#include "PBGateAccelerationBumperEffect.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateAccelerationField.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"

UPBGateAccelerationBumperEffect::UPBGateAccelerationBumperEffect()
{
	SummonActorClass = APBGateAccelerationField::StaticClass();
	IsDestroyOnFinished = false;
}

void UPBGateAccelerationBumperEffect::Initialize(APBModularBumperBase* InOwnerBumper)
{
	Super::Initialize(InOwnerBumper);
	ConfigureAccelerationField();
}

void UPBGateAccelerationBumperEffect::ActivateEffect(
	APBModularBumperBase* Bumper,
	APBBallBase* Ball)
{
	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration skipped because Power is invalid. Power=%.2f"),
			EffectData.Power);
		FinishEffect();
		return;
	}

	if (!IsValid(Bumper) || !IsValid(Ball) || !IsValid(SpawnedSummonActor))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration activation context is invalid. Bumper=%s Ball=%s Field=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(Ball),
			*GetNameSafe(SpawnedSummonActor));
		FinishEffect();
		return;
	}

	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	if (!IsValid(ActiveTrigger))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration skipped because the active Trigger is missing. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	const FTransform FieldTransform = SpawnOffset * ActiveTrigger->GetActorTransform();
	SpawnedSummonActor->SetActorTransform(FieldTransform);
	if (!ConfigureAccelerationField())
	{
		FinishEffect();
		return;
	}

	Super::ActivateEffect(Bumper, Ball);
}

bool UPBGateAccelerationBumperEffect::ConfigureAccelerationField() const
{
	APBGateAccelerationField* AccelerationField = Cast<APBGateAccelerationField>(SpawnedSummonActor);
	if (!IsValid(AccelerationField))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration field is unavailable. Effect=%s SummonActor=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SpawnedSummonActor));
		return false;
	}

	AccelerationField->ConfigureField(EffectData.Power, FieldDuration);
	return true;
}
