// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperSummonActor.h"

#include "Components/SceneComponent.h"

APBBumperSummonActor::APBBumperSummonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBumperSummonActor::StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	OnStartAction(Bumper, Ball);
}

void APBBumperSummonActor::FinishAction()
{
	OnSummonActionFinished.Broadcast(this);
}

void APBBumperSummonActor::DeactivateSummon()
{
	OnDeactivateSummon();
}
