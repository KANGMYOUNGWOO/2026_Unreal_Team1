// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTurretSummonActor.h"

APBTurretSummonActor::APBTurretSummonActor()
{
}

void APBTurretSummonActor::StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	Super::StartAction(Bumper, Ball);

	OnTurretActivated(Bumper, Ball);
}

void APBTurretSummonActor::DeactivateSummon()
{
	Super::DeactivateSummon();
}
