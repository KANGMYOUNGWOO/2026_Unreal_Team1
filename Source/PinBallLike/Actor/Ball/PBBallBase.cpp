// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallBase.h"

#include "Component/PBBallCollisionComponent.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Common/Componenet/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Common/Componenet/Resource/PBResourceTypes.h"
#include "PinBallLike/Actor/Common/Componenet/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/Common/Componenet/Stat/PBStatTypes.h"
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

	// Movement
	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	
	// Stat
	StatComponent = CreateDefaultSubobject<UPBBaseStatComponent>(TEXT("StatComponent"));

	// Resource
	ResourceComponent = CreateDefaultSubobject<UPBBaseResourceComponent>(TEXT("ResourceComponent"));

	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));

	// Collision Rule
	CollisionComponent = CreateDefaultSubobject<UPBBallCollisionComponent>(TEXT("CollisionComponent"));

	DefaultStats.Emplace(PBStatNames::Mass, 1);
	DefaultStats.Emplace(PBStatNames::Bounciness, 30);
	DefaultStats.Emplace(PBStatNames::Size, 25);
	DefaultStats.Emplace(PBStatNames::Attack, 0);
	DefaultStats.Emplace(PBStatNames::StaggerPower, 0);
	DefaultStats.Emplace(PBStatNames::ManaRegen, 0);

	DefaultResources.Emplace(PBResourceNames::Health, 100.0f, 100.0f, 0.0f);
	DefaultResources.Emplace(PBResourceNames::Mana, 50.0f, 50.0f, 1.0f);
	
	InitializeDefaultStats();
	InitializeDefaultResources();

	DisplayName = "BaseBall";
}

void APBBallBase::LaunchBall(const FVector Impulse)
{
	PhysicsComponent->Launch(Impulse, Impulse.Size2D());
}

void APBBallBase::AddVelocity(const FVector VelocityToAdd)
{
	UE_LOG(LogTemp, Warning, TEXT("AddVelocity %s"), *VelocityToAdd.ToString());
	PhysicsComponent->AddVelocity(VelocityToAdd);
}

void APBBallBase::ApplyStatData(const TArray<FPBBallStatData>& StatData)
{
	for (const FPBBallStatData& Stat : StatData)
	{
		if (Stat.StatName.IsNone())
		{
			continue;
		}

		SetStat(Stat.StatName, Stat.Value);
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

		SetResource(Resource.ResourceName, Resource.Current, Resource.Max);
		SetResourceRegenPerSecond(Resource.ResourceName, Resource.RegenPerSecond);
	}
}

bool APBBallBase::HasStat(FName StatName) const
{
	return StatComponent->HasStat(StatName);
}

int32 APBBallBase::GetStat(FName StatName) const
{
	return StatComponent->GetStat(StatName);
}

void APBBallBase::SetStat(FName StatName, int32 Value)
{
	StatComponent->SetStat(StatName, Value);
	ApplyStatToComponents(StatName);
}

void APBBallBase::ApplyStat(FName StatName, int32 Delta)
{
	if (Delta == 0)
	{
		return;
	}
	
	StatComponent->ApplyStat(StatName, Delta);
	ApplyStatToComponents(StatName);
}

bool APBBallBase::HasResource(FName ResourceName) const
{
	return ResourceComponent->HasResource(ResourceName);
}

float APBBallBase::GetResourceCurrent(FName ResourceName) const
{
	return ResourceComponent->GetCurrent(ResourceName);
}

float APBBallBase::GetResourceMax(FName ResourceName) const
{
	return ResourceComponent->GetMax(ResourceName);
}

float APBBallBase::GetResourceRatio(FName ResourceName) const
{
	return ResourceComponent->GetRatio(ResourceName);
}

void APBBallBase::SetResource(FName ResourceName, float Current, float Max)
{
	ResourceComponent->SetResource(ResourceName, Current, Max);
}

void APBBallBase::SetResourceCurrent(FName ResourceName, float Value)
{
	ResourceComponent->SetCurrent(ResourceName, Value);
}

void APBBallBase::SetResourceMax(FName ResourceName, float Value, bool bFillCurrent)
{
	ResourceComponent->SetMax(ResourceName, Value, bFillCurrent);
}

void APBBallBase::SetResourceRegenPerSecond(FName ResourceName, float Value)
{
	ResourceComponent->SetRegenPerSecond(ResourceName, Value);
}

void APBBallBase::ApplyResourceDelta(FName ResourceName, float Delta)
{
	ResourceComponent->ApplyDelta(ResourceName, Delta);
}

bool APBBallBase::CanConsumeResource(FName ResourceName, float Cost) const
{
	return ResourceComponent->CanConsume(ResourceName, Cost);
}

bool APBBallBase::ConsumeResource(FName ResourceName, float Cost)
{
	return ResourceComponent->Consume(ResourceName, Cost);
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

FVector APBBallBase::GetVelocity() const
{
	return PhysicsComponent->GetVelocity();
}

void APBBallBase::AddImpulse(FVector Impulse)
{
	PhysicsComponent->AddImpulse(Impulse);
}

void APBBallBase::StopMovement()
{
	PhysicsComponent->Stop();
}

void APBBallBase::PauseMovement()
{
	PhysicsComponent->PauseMovement();
}

void APBBallBase::ResumeMovement()
{
	PhysicsComponent->ResumeMovement();
}

bool APBBallBase::IsMovementPaused() const
{
	return PhysicsComponent->IsMovementPaused();
}

void APBBallBase::TakeDamage(int32 Damage)
{
	if (Damage <= 0)
	{
		return;
	}

	ApplyResourceDelta(PBResourceNames::Health, -Damage);
}

bool APBBallBase::IsDead() const
{
	return GetResourceCurrent(PBResourceNames::Health) <= 0.0f;
}

void APBBallBase::InitializeDefaultStats()
{
	ApplyStatData(DefaultStats);
}

void APBBallBase::InitializeDefaultResources()
{
	ApplyResourceData(DefaultResources);
}

void APBBallBase::ApplyStatToComponents(FName StatName)
{
	const int32 StatValue = GetStat(StatName);

	if (StatName == PBStatNames::Mass)
	{
		PhysicsComponent->SetMass(static_cast<float>(StatValue));
	}
	else if (StatName == PBStatNames::Bounciness)
	{
		PhysicsComponent->SetBounceDamping(static_cast<float>(StatValue) / 100.0f);
	}
	else if (StatName == PBStatNames::Size)
	{
		CollisionSphere->SetSphereRadius(FMath::Max(static_cast<float>(StatValue), 1.0f), true);
	}
}
