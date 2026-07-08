// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallHitReactionComponent.h"

#include "PBBallPhysicsComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"


UPBBallHitReactionComponent::UPBBallHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallHitReactionComponent::InitializeDependencies(
	UPBBallPhysicsComponent* InPhysicsComponent,
	IStatProvider* InStatProvider,
	IDamageable* InOwnerDamageable)
{
	PhysicsComponent = InPhysicsComponent;
	StatProvider = InStatProvider;
	OwnerDamageable = InOwnerDamageable;
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
	if (!Owner || !OtherActor || OtherActor == Owner || WasContactProcessedThisFrame(OtherActor))
	{
		return;
	}
	
	IBossInterface* Boss = Cast<IBossInterface>(OtherActor);
	if (!Boss)
	{
		return;
	}

	const int32 Damage = StatProvider ? StatProvider->GetStat(PBStatNames::Attack) : 0;
	Boss->DamageToBoss(Damage);

	MarkContactProcessed(OtherActor);

	if (OwnerDamageable && !OwnerDamageable->IsDead())
	{
		OwnerDamageable->TakeDamage(1);
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
