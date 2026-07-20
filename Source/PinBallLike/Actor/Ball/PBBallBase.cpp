// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallBase.h"
#include  "PinBallLike/Relic/PBRelicCalculator.h"
#include "Component/PBBallHitReactionComponent.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallEffectRuntimeComponent.h"
#include "Component/PBBallPhysicsComponent.h"
#include "Component/PBBallResourceComponent.h"
#include "Component/PBBallSkillComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
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
	ResourceComponent = CreateDefaultSubobject<UPBBallResourceComponent>(TEXT("ResourceComponent"));

	// StatusEffect
	StatusEffectComponent = CreateDefaultSubobject<UPBStatusEffectComponent>(TEXT("StatusEffectComponent"));

	// Effect
	EffectRuntimeComponent = CreateDefaultSubobject<UPBBallEffectRuntimeComponent>(TEXT("EffectRuntimeComponent"));
	
	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));

	// Skill
	SkillComponent = CreateDefaultSubobject<UPBBallSkillComponent>(TEXT("SkillComponent"));
	
	// Physics
	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), StatComponent.Get());
	
	// Hit Reaction
	HitReactionComponent = CreateDefaultSubobject<UPBBallHitReactionComponent>(TEXT("HitReactionComponent"));
	HitReactionComponent->InitializeDependencies(
		PhysicsComponent.Get(),
		StatComponent.Get(),
		ResourceComponent.Get());
}

void APBBallBase::ApplyStatData(const TArray<FPBStatData>& StatData)
{
	for (const FPBStatData& Stat : StatData)
	{
		if (Stat.StatName.IsNone())
		{
			continue;
		}
		StatComponent->SetStat(Stat.StatName, Stat.Value);
	}
}


void APBBallBase::ApplyResourceData(const TArray<FPBResourceData>& ResourceData)
{
	for (const FPBResourceData& Resource : ResourceData)
	{
		if (Resource.ResourceName.IsNone())
		{
			continue;   
		}
		ResourceComponent->SetResource(Resource.ResourceName, Resource.Current, Resource.Max);
		ResourceComponent->SetResourceRegenPerSecond(Resource.ResourceName, Resource.RegenPerSecond);
	}
}

void APBBallBase::InitializeFromBallInstanceData(const FPBBallInstanceData& InBallInstanceData)
{
	BallInstanceData = InBallInstanceData;
	ApplyStatData(BallInstanceData.BaseStats);
	ApplyResourceData(BallInstanceData.BaseResources);
	if (SkillComponent)
	{
		SkillComponent->InitializeSkill(BallInstanceData);
	}
}

void APBBallBase::SetCombatRole(EPBBallPartyRole NewCombatRole)
{
	CombatRole = NewCombatRole;

	const bool bLeader = CombatRole == EPBBallPartyRole::Leader;
	if (PhysicsComponent)
	{
		if (bLeader)
		{
			PhysicsComponent->ResumeMovement();
		}
		else
		{
			PhysicsComponent->StopMovement();
			PhysicsComponent->PauseMovement();
		}
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(bLeader ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		CollisionSphere->SetGenerateOverlapEvents(bLeader);
		CollisionSphere->SetNotifyRigidBodyCollision(bLeader);
	}
}

bool APBBallBase::TryActivateSkill()
{
	if (IsHidden() || CombatRole == EPBBallPartyRole::None)
	{
		return false;
	}
	return SkillComponent && SkillComponent->TryActivateSkill();
}

void APBBallBase::RefreshRelicStats(const UPBRelicCalculator* RelicCalculator)
{
	if (!IsValid(RelicCalculator) ||
		!IsValid(StatComponent))
	{
		return;
	}

	for (const FPBStatData& BaseStat : BallInstanceData.BaseStats)
	{
		if (BaseStat.StatName.IsNone())
		{
			continue;
		}

		const int32 FinalValue =
			RelicCalculator->CalculateBallStat(
				BaseStat.StatName,
				BaseStat.Value);

		StatComponent->SetStat(
			BaseStat.StatName,
			FinalValue);
	}
	
}

void APBBallBase::BeginPlay()
{
	Super::BeginPlay();
}
