// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperPositionAnchor.h"

#include "Components/SceneComponent.h"

APBBumperPositionAnchor::APBBumperPositionAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

EPBBumperPositionId APBBumperPositionAnchor::GetPositionId() const
{
	return PositionId;
}
