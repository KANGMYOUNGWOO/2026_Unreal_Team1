// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallCollisionComponent.h"

#include "PBBallPhysicsComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PinBallLike/Interface/BallComboSource.h"
#include "PinBallLike/Interface/BallDamageSource.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Damageable.h"


UPBBallCollisionComponent::UPBBallCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	MovementComponent = Owner->FindComponentByClass<UPBBallPhysicsComponent>();
	if (MovementComponent)
	{
		MovementComponent->OnBallMovementHit.AddDynamic(this, &UPBBallCollisionComponent::HandleMovementHit);
	}

	CollisionPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (CollisionPrimitive)
	{
		CollisionPrimitive->OnComponentBeginOverlap.AddDynamic(
			this,
			&UPBBallCollisionComponent::HandleCollisionBeginOverlap);
	}
}

void UPBBallCollisionComponent::HandleMovementHit(const FHitResult& Hit)
{
	ProcessBallContact(Hit.GetActor());
}

void UPBBallCollisionComponent::HandleCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ProcessBallContact(OtherActor);
}

void UPBBallCollisionComponent::ProcessBallContact(AActor* OtherActor)
{
	AActor* Owner = GetOwner();
	if (!Owner || !OtherActor || OtherActor == Owner || WasContactProcessedThisFrame(OtherActor))
	{
		return;
	}

	MarkContactProcessed(OtherActor);

	if (IBallDamageSource* DamageSource = Cast<IBallDamageSource>(OtherActor))
	{
		if (IDamageable* Damagable = Cast<IDamageable>(Owner))
		{
			Damagable->TakeDamage(DamageSource->GetBallDamage());
		}
	}

	if (IBallComboSource* ComboSource = Cast<IBallComboSource>(OtherActor))
	{
		if (IComboable* Comboable = Cast<IComboable>(Owner))
		{
			Comboable->AddCombo(ComboSource->GetBallComboDelta());
		}
	}
}

bool UPBBallCollisionComponent::WasContactProcessedThisFrame(AActor* OtherActor) const
{
	const UWorld* World = GetWorld();
	return World
		&& FMath::IsNearlyEqual(ProcessedContactTime, World->GetTimeSeconds())
		&& ProcessedContactActors.Contains(TObjectKey<AActor>(OtherActor));
}

void UPBBallCollisionComponent::MarkContactProcessed(AActor* OtherActor)
{
	if (const UWorld* World = GetWorld())
	{
		const double CurrentTime = World->GetTimeSeconds();
		if (!FMath::IsNearlyEqual(ProcessedContactTime, CurrentTime))
		{
			ProcessedContactActors.Reset();
			ProcessedContactTime = CurrentTime;
		}
	}

	ProcessedContactActors.Add(TObjectKey<AActor>(OtherActor));
}
