// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTurretSummonActor.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"

APBTurretSummonActor::APBTurretSummonActor()
{
}

void APBTurretSummonActor::StartActionForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	Super::StartActionForActor(Bumper, InteractionActor);
	OnTurretActivatedForActor(Bumper, InteractionActor);

	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		OnTurretActivated(Bumper, Ball);
	}
}

void APBTurretSummonActor::DeactivateSummon()
{
	Super::DeactivateSummon();
}
