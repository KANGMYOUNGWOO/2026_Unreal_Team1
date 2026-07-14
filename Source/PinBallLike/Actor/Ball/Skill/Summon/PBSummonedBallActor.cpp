// Fill out your copyright notice in the Description page of Project Settings.

#include "PBSummonedBallActor.h"

#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Component/PBBallHitReactionComponent.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"

APBSummonedBallActor::APBSummonedBallActor()
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

	StatComponent = CreateDefaultSubobject<UPBBaseStatComponent>(TEXT("StatComponent"));

	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), StatComponent.Get());

	HitReactionComponent = CreateDefaultSubobject<UPBBallHitReactionComponent>(TEXT("HitReactionComponent"));
	HitReactionComponent->InitializeDependencies(PhysicsComponent.Get(), StatComponent.Get(), nullptr);
}

void APBSummonedBallActor::InitializeFromSourceBall(APBBallBase* InSourceBall)
{
	if (!IsValid(InSourceBall))
	{
		return;
	}

	SourceBall = InSourceBall;
	const UPBBaseStatComponent* SourceStatComponent =
		InSourceBall->FindComponentByClass<UPBBaseStatComponent>();
	CopyStat(SourceStatComponent, PBStatNames::Attack);
	CopyStat(SourceStatComponent, PBStatNames::StaggerPower);
	CopyStat(SourceStatComponent, PBStatNames::Mass);
	CopyStat(SourceStatComponent, PBStatNames::Bounciness);
	CopyStat(SourceStatComponent, PBStatNames::Size);
	CopyStat(SourceStatComponent, PBStatNames::ManaRegen);

	if (const UPBBallPhysicsComponent* SourcePhysicsComponent =
		InSourceBall->FindComponentByClass<UPBBallPhysicsComponent>())
	{
		PhysicsComponent->SetVelocity(SourcePhysicsComponent->GetVelocity());
	}
}

void APBSummonedBallActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(SourceBall))
	{
		InitializeFromSourceBall(SourceBall);
	}

	if (LifeTime > 0.0f)
	{
		SetLifeSpan(LifeTime);
	}
}

void APBSummonedBallActor::CopyStat(
	const UPBBaseStatComponent* SourceStatComponent,
	const FName StatName)
{
	if (SourceStatComponent && SourceStatComponent->HasStat(StatName))
	{
		StatComponent->SetStat(StatName, SourceStatComponent->GetStat(StatName));
	}
}
