// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallBase.h"

#include "Component/PBBallHitReactionComponent.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBResourceTypes.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallGaugeComponent.h"
#include "Component/PBBallStatComponent.h"
#include "Component/PBBallMovementComponent.h"
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
	HitReactionComponent->InitializeDependencies(PhysicsComponent.Get(), ResourceComponent.Get(), ComboComponent.Get());

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

	// Movement
	MovementComponent = CreateDefaultSubobject<UPBBallMovementComponent>(TEXT("MovementComponent"));
	
	// Stat
	StatComponent = CreateDefaultSubobject<UPBBallStatComponent>(TEXT("StatComponent"));

	// Gauge
	GaugeComponent = CreateDefaultSubobject<UPBBallGaugeComponent>(TEXT("GaugeComponent"));

	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));
	
	DisplayName = "BaseBall";
}

void APBBallBase::LaunchBall(const FVector Impulse)
{
	MovementComponent->Launch(Impulse, Impulse.Size2D());
}

void APBBallBase::AddVelocity(const FVector VelocityToAdd)
{
	UE_LOG(LogTemp, Warning, TEXT("AddVelocity %s"), *VelocityToAdd.ToString());
	MovementComponent->AddVelocity(VelocityToAdd);
}

int32 APBBallBase::GetStat(EBallStatType Type) const
{
	return StatComponent->GetStat(Type);
}

void APBBallBase::ApplyStat(EBallStatType Type, int32 Delta)
{
	if (Delta == 0)
	{
		return;
	}
	
	StatComponent->ApplyStat(Type, Delta);
}

bool APBBallBase::HasGauge(EBallGaugeType Type) const
{
	return GaugeComponent->HasGauge(Type);
}

float APBBallBase::GetGaugeCurrent(EBallGaugeType Type) const
{
	return GaugeComponent->GetCurrent(Type);
}

float APBBallBase::GetGaugeMax(EBallGaugeType Type) const
{
	return GaugeComponent->GetMax(Type);
}

float APBBallBase::GetGaugeRatio(EBallGaugeType Type) const
{
	return GaugeComponent->GetRatio(Type);
}

void APBBallBase::SetGauge(EBallGaugeType Type, float Current, float Max)
{
	GaugeComponent->SetGauge(Type, Current, Max);
}

void APBBallBase::SetGaugeCurrent(EBallGaugeType Type, float Value)
{
	GaugeComponent->SetCurrent(Type, Value);
}

void APBBallBase::SetGaugeMax(EBallGaugeType Type, float Value, bool bFillCurrent)
{
	GaugeComponent->SetMax(Type, Value, bFillCurrent);
}

void APBBallBase::SetGaugeRegenPerSecond(EBallGaugeType Type, float Value)
{
	GaugeComponent->SetRegenPerSecond(Type, Value);
}

void APBBallBase::ApplyGaugeDelta(EBallGaugeType Type, float Delta)
{
	GaugeComponent->ApplyDelta(Type, Delta);
}

bool APBBallBase::CanConsumeGauge(EBallGaugeType Type, float Cost) const
{
	return GaugeComponent->CanConsume(Type, Cost);
}

bool APBBallBase::ConsumeGauge(EBallGaugeType Type, float Cost)
{
	return GaugeComponent->Consume(Type, Cost);
}

int32 APBBallBase::GetCombo() const
{
	return ComboComponent->GetCombo();
}

void APBBallBase::SetCombo(int32 Value)
{
	ComboComponent->SetCombo(Value);
}

void APBBallBase::AddCombo(int32 Delta)
{
	ComboComponent->AddCombo(Delta);
}

bool APBBallBase::TryConsumeCombo(int32 Cost)
{
	return ComboComponent->TryConsumeCombo(Cost);
}

void APBBallBase::ResetCombo()
{
	ComboComponent->ResetCombo();
}

void APBBallBase::TakeDamage(int32 Damage)
{
	if (Damage <= 0)
	{
		return;
	}

	ApplyGaugeDelta(EBallGaugeType::EGT_HP, -Damage);
}

bool APBBallBase::IsDead() const
{
	return GetGaugeCurrent(EBallGaugeType::EGT_HP) <= 0.0f;
}
