// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallBase.h"

#include "Component/PBBallHitReactionComponent.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBResourceTypes.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"

APBBallBase::APBBallBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(25.0f);
	CollisionSphere->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetEnableGravity(false);
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	
	// Stat
	StatComponent = CreateDefaultSubobject<UPBBaseStatComponent>(TEXT("StatComponent"));
	
	// Resource
	ResourceComponent = CreateDefaultSubobject<UPBBaseResourceComponent>(TEXT("ResourceComponent"));
	
	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));
	
	// Physics
	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), StatComponent.Get());
	
	// Hit Reaction
	HitReactionComponent = CreateDefaultSubobject<UPBBallHitReactionComponent>(TEXT("HitReactionComponent"));
	HitReactionComponent->InitializeDependencies(PhysicsComponent.Get(), StatComponent.Get());

	// @Test
	DefaultStats.Emplace(PBStatNames::Mass, 1);
	DefaultStats.Emplace(PBStatNames::Bounciness, 30);
	DefaultStats.Emplace(PBStatNames::Size, 25);
	DefaultStats.Emplace(PBStatNames::Attack, 0);
	DefaultStats.Emplace(PBStatNames::StaggerPower, 0);
	DefaultStats.Emplace(PBStatNames::ManaRegen, 0);

	DefaultResources.Emplace(PBResourceNames::Health, 100.0f, 100.0f, 0.0f);
	DefaultResources.Emplace(PBResourceNames::Mana, 50.0f, 50.0f, 1.0f);

	DisplayName = "BaseBall";
}

void APBBallBase::ApplyStatData(const TArray<FPBBallStatData>& StatData)
{
	for (const FPBBallStatData& Stat : StatData)
	{
		if (Stat.StatName.IsNone())
		{
			continue;
		}
		StatComponent->SetStat(Stat.StatName, Stat.Value);
	}
}


void APBBallBase::ApplyResourceData(const TArray<FPBBallResourceData>& ResourceData)
{
	for (const FPBBallResourceData& Resource : ResourceData)
	{
		if (Resource.ResourceName.IsNone())
		{
			continue;   
		}
		ResourceComponent->SetResource(Resource.ResourceName, Resource.Current, Resource.Max);
		ResourceComponent->SetResourceRegenPerSecond(Resource.ResourceName, Resource.RegenPerSecond);
	}
}

void APBBallBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeDefaultStats();
	InitializeDefaultResources();
}

void APBBallBase::InitializeDefaultStats()
{
	ApplyStatData(DefaultStats);
}

void APBBallBase::InitializeDefaultResources()
{
	ApplyResourceData(DefaultResources);
}
