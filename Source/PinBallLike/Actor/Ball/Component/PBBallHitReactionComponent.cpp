// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallHitReactionComponent.h"

#include "PBBallPhysicsComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PinBallLike/Interface/BallComboSource.h"
#include "PinBallLike/Interface/BallDamageSource.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Damageable.h"


UPBBallHitReactionComponent::UPBBallHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallHitReactionComponent::InitializeDependencies(
	UPBBallPhysicsComponent* InPhysicsComponent,
	IDamageable* InDamageable,
	IComboable* InComboable)
{
	PhysicsComponent = InPhysicsComponent;
	Damageable = InDamageable;
	Comboable = InComboable;
}

void UPBBallHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PhysicsComponent)
	{
		PhysicsComponent->OnBallMovementHit.AddDynamic(this, &UPBBallHitReactionComponent::HandleMovementHit);
	}
}

void UPBBallHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->OnBallMovementHit.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UPBBallHitReactionComponent::HandleMovementHit(const FHitResult& Hit)
{
	ProcessBallContact(Hit.GetActor());
}

void UPBBallHitReactionComponent::ProcessBallContact(AActor* OtherActor)
{
	AActor* Owner = GetOwner();
	// 이미 이번 프레임에 처리한 액터인 경우 무시합니다.
	if (!Owner || !OtherActor || OtherActor == Owner || WasContactProcessedThisFrame(OtherActor))
	{
		return;
	}

	// 현재 프레임에서 이 액터와의 충돌을 처리했음을 기록합니다.
	MarkContactProcessed(OtherActor);

	if (IBallDamageSource* DamageSource = Cast<IBallDamageSource>(OtherActor))
	{
		if (Damageable)
		{
			Damageable->TakeDamage(DamageSource->GetBallDamage());
		}
	}

	if (IBallComboSource* ComboSource = Cast<IBallComboSource>(OtherActor))
	{
		if (Comboable)
		{
			Comboable->AddCombo(ComboSource->GetBallComboDelta());
		}
	}
}

bool UPBBallHitReactionComponent::WasContactProcessedThisFrame(AActor* OtherActor) const
{
	const UWorld* World = GetWorld();
	// 월드가 유효하고, 마지막 처리 시간과 현재 시간이 거의 같으며, 처리된 액터 목록에 OtherActor가 포함되어 있는지 확인합니다.
	return World
		&& FMath::IsNearlyEqual(ProcessedContactTime, World->GetTimeSeconds())
		&& ProcessedContactActors.Contains(TObjectKey<AActor>(OtherActor));
}

void UPBBallHitReactionComponent::MarkContactProcessed(AActor* OtherActor)
{
	if (const UWorld* World = GetWorld())
	{
		const double CurrentTime = World->GetTimeSeconds();
		// 마지막 처리 시간과 현재 시간이 다르다면, 새로운 프레임으로 간주하고 처리된 액터 목록을 초기화합니다.
		if (!FMath::IsNearlyEqual(ProcessedContactTime, CurrentTime))
		{
			ProcessedContactActors.Reset();
			ProcessedContactTime = CurrentTime;
		}
	}

	ProcessedContactActors.Add(TObjectKey<AActor>(OtherActor));
}
