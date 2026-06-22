// Fill out your copyright notice in the Description page of Project Settings.


#include "BallBase.h"

#include "Component/PBBallComboComponent.h"
#include "Component/PBBallGaugeComponent.h"
#include "Component/PBBallStatComponent.h"
#include "Component/PinballBallMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"

ABallBase::ABallBase()
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
	MovementComponent = CreateDefaultSubobject<UPinballBallMovementComponent>(TEXT("MovementComponent"));
	
	// Stat
	StatComponent = CreateDefaultSubobject<UPBBallStatComponent>(TEXT("StatComponent"));

	// Gauge
	GaugeComponent = CreateDefaultSubobject<UPBBallGaugeComponent>(TEXT("GaugeComponent"));

	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));
	
	DisplayName = "BaseBall";
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

int32 ABallBase::GetStat(EBallStatType Type) const
{
	return StatComponent->GetStat(Type);
}

void ABallBase::ApplyStat(EBallStatType Type, int32 Delta)
{
	if (Delta == 0)
	{
		return;
	}
	
	StatComponent->ApplyStat(Type, Delta);
}

bool ABallBase::HasGauge(EBallGaugeType Type) const
{
	return GaugeComponent->HasGauge(Type);
}

float ABallBase::GetGaugeCurrent(EBallGaugeType Type) const
{
	return GaugeComponent->GetCurrent(Type);
}

float ABallBase::GetGaugeMax(EBallGaugeType Type) const
{
	return GaugeComponent->GetMax(Type);
}

float ABallBase::GetGaugeRatio(EBallGaugeType Type) const
{
	return GaugeComponent->GetRatio(Type);
}

void ABallBase::SetGauge(EBallGaugeType Type, float Current, float Max)
{
	GaugeComponent->SetGauge(Type, Current, Max);
}

void ABallBase::SetGaugeCurrent(EBallGaugeType Type, float Value)
{
	GaugeComponent->SetCurrent(Type, Value);
}

void ABallBase::SetGaugeMax(EBallGaugeType Type, float Value, bool bFillCurrent)
{
	GaugeComponent->SetMax(Type, Value, bFillCurrent);
}

void ABallBase::SetGaugeRegenPerSecond(EBallGaugeType Type, float Value)
{
	GaugeComponent->SetRegenPerSecond(Type, Value);
}

void ABallBase::ApplyGaugeDelta(EBallGaugeType Type, float Delta)
{
	GaugeComponent->ApplyDelta(Type, Delta);
}

bool ABallBase::CanConsumeGauge(EBallGaugeType Type, float Cost) const
{
	return GaugeComponent->CanConsume(Type, Cost);
}

bool ABallBase::ConsumeGauge(EBallGaugeType Type, float Cost)
{
	return GaugeComponent->Consume(Type, Cost);
}

int32 ABallBase::GetCombo() const
{
	return ComboComponent->GetCombo();
}

void ABallBase::SetCombo(int32 Value)
{
	ComboComponent->SetCombo(Value);
}

void ABallBase::AddCombo(int32 Delta)
{
	ComboComponent->AddCombo(Delta);
}

bool ABallBase::TryConsumeCombo(int32 Cost)
{
	return ComboComponent->TryConsumeCombo(Cost);
}

void ABallBase::ResetCombo()
{
	ComboComponent->ResetCombo();
}

void ABallBase::TakeDamage(int32 Damage)
{
	if (Damage <= 0)
	{
		return;
	}

	ApplyGaugeDelta(EBallGaugeType::EGT_HP, -Damage);
}

bool ABallBase::IsDead() const
{
	return GetGaugeCurrent(EBallGaugeType::EGT_HP) <= 0.0f;
}
