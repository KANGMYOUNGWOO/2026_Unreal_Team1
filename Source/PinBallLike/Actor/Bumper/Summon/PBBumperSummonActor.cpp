// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperSummonActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"

APBBumperSummonActor::APBBumperSummonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBumperSummonActor::StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	StartActionForActor(Bumper, Ball);
}

void APBBumperSummonActor::StartActionForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	OnStartActionForActor(Bumper, InteractionActor);
	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		OnStartAction(Bumper, Ball);
	}
}

void APBBumperSummonActor::FinishAction()
{
	OnSummonActionFinished.Broadcast(this);
}

void APBBumperSummonActor::DeactivateSummon()
{
	OnDeactivateSummon();
}
