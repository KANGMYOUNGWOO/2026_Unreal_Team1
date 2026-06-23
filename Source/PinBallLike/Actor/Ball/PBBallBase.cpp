// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallBase.h"

#include "Component/PBBallCollisionComponent.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallGaugeComponent.h"
#include "Component/PBBallStatComponent.h"
#include "Component/PBBallPhysicsComponent.h"
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
	StatComponent = CreateDefaultSubobject<UPBBallStatComponent>(TEXT("StatComponent"));

	// Gauge
	GaugeComponent = CreateDefaultSubobject<UPBBallGaugeComponent>(TEXT("GaugeComponent"));

	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));

	// Collision Rule
	CollisionComponent = CreateDefaultSubobject<UPBBallCollisionComponent>(TEXT("CollisionComponent"));
	
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
	ApplyStatToComponents(Type);
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

	ApplyGaugeDelta(EBallGaugeType::EGT_HP, -Damage);
}

bool APBBallBase::IsDead() const
{
	return GetGaugeCurrent(EBallGaugeType::EGT_HP) <= 0.0f;
}

void APBBallBase::ApplyStatToComponents(EBallStatType Type)
{
	const int32 StatValue = GetStat(Type);

	switch (Type)
	{
	case EBallStatType::EST_WEIGHT:
		PhysicsComponent->SetMass(static_cast<float>(StatValue));
		break;

	case EBallStatType::EST_BOUNCINESS:
		PhysicsComponent->SetBounceDamping(static_cast<float>(StatValue) / 100.0f);
		break;

	case EBallStatType::EST_SIZE:
		CollisionSphere->SetSphereRadius(FMath::Max(static_cast<float>(StatValue), 1.0f), true);
		break;

	default:
		break;
	}
}
