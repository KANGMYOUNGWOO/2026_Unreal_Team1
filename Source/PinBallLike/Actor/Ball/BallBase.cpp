// Fill out your copyright notice in the Description page of Project Settings.


#include "BallBase.h"

#include "Component/PinballBallMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"

ABallBase::ABallBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(25.0f);
	CollisionSphere->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetEnableGravity(false);
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	MovementComponent = CreateDefaultSubobject<UPinballBallMovementComponent>(TEXT("MovementComponent"));
}

void ABallBase::LaunchBall(const FVector Impulse)
{
	MovementComponent->Launch(Impulse, Impulse.Size2D());
}

void ABallBase::AddVelocity(const FVector VelocityToAdd)
{
	UE_LOG(LogTemp, Warning, TEXT("AddVelocity %s"), *VelocityToAdd.ToString());
	MovementComponent->AddVelocity(VelocityToAdd);
}
