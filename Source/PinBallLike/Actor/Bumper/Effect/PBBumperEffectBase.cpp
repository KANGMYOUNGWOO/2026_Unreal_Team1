// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperEffectBase.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"

void UPBBumperEffectBase::InitializeEffect(
	APBModularBumperBase* InOwnerBumper,
	const FPBBumperEffectRow& InEffectData)
{
	EffectData = InEffectData;
	Initialize(InOwnerBumper);
}

void UPBBumperEffectBase::Initialize(APBModularBumperBase* InOwnerBumper)
{
	OwnerBumper = InOwnerBumper;
}

void UPBBumperEffectBase::ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	ActivateEffectForActor(Bumper, Ball);
}

void UPBBumperEffectBase::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	FinishEffect();
}

void UPBBumperEffectBase::FinishEffect()
{
	if (OwnerBumper.IsValid())
	{
		OwnerBumper->FinishActivation();
	}
}

FPBBumperEffectRow UPBBumperEffectBase::GetEffectData() const
{
	return EffectData;
}
