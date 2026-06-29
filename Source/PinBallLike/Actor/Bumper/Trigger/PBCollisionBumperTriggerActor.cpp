// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCollisionBumperTriggerActor.h"

#include "Components/PrimitiveComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperReactionComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBCollisionBumperTriggerActor::APBCollisionBumperTriggerActor()
{
	TriggerType = EPBBumperTriggerType::HitCount;
	ReactionComponent = CreateDefaultSubobject<UPBBumperReactionComponent>(TEXT("ReactionComponent"));
}

void APBCollisionBumperTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	RegisterCollisionAreas();
}

void APBCollisionBumperTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UPrimitiveComponent* CollisionArea : CollisionAreas)
	{
		if (!IsValid(CollisionArea))
		{
			continue;
		}

		CollisionArea->OnComponentHit.RemoveDynamic(
			this,
			&APBCollisionBumperTriggerActor::HandleComponentHit);
	}

	for (UPrimitiveComponent* TriggerArea : TriggerAreas)
	{
		if (!IsValid(TriggerArea))
		{
			continue;
		}

		TriggerArea->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&APBCollisionBumperTriggerActor::HandleTriggerBeginOverlap);
		TriggerArea->OnComponentEndOverlap.RemoveDynamic(
			this,
			&APBCollisionBumperTriggerActor::HandleTriggerEndOverlap);
	}

	CollisionAreas.Reset();
	TriggerAreas.Reset();
	TriggeringBallOverlapCounts.Reset();

	Super::EndPlay(EndPlayReason);
}

void APBCollisionBumperTriggerActor::RegisterCollisionAreas()
{
	// BP에서 만든 충돌 영역과 유효 판정 영역을 각각 태그 기준으로 찾는다.
	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		CollisionAreaTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		SetupCollisionArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}

	TArray<UActorComponent*> TaggedTriggerComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		TriggerAreaTag);

	for (UActorComponent* TaggedComponent : TaggedTriggerComponents)
	{
		SetupTriggerArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}
}

void APBCollisionBumperTriggerActor::SetupCollisionArea(UPrimitiveComponent* CollisionArea)
{
	if (!IsValid(CollisionArea) || CollisionAreas.Contains(CollisionArea))
	{
		return;
	}

	// 같은 컴포넌트가 중복 등록되어도 Hit가 한 번만 오도록 바인딩한다.
	CollisionArea->SetNotifyRigidBodyCollision(true);
	CollisionArea->OnComponentHit.AddUniqueDynamic(
		this,
		&APBCollisionBumperTriggerActor::HandleComponentHit);
	CollisionAreas.Add(CollisionArea);
}

void APBCollisionBumperTriggerActor::SetupTriggerArea(UPrimitiveComponent* TriggerArea)
{
	if (!IsValid(TriggerArea) || TriggerAreas.Contains(TriggerArea))
	{
		return;
	}

	// 유효 판정 영역 안에 들어온 볼만 Hit 발동 대상으로 인정한다.
	TriggerArea->SetGenerateOverlapEvents(true);

	TriggerArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBCollisionBumperTriggerActor::HandleTriggerBeginOverlap);
	TriggerArea->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APBCollisionBumperTriggerActor::HandleTriggerEndOverlap);
	
	TriggerAreas.Add(TriggerArea);
}

bool APBCollisionBumperTriggerActor::IsBallInTriggerArea(APBBallBase* Ball) const
{
	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	const int32* OverlapCount = TriggeringBallOverlapCounts.Find(BallKey);
	return OverlapCount != nullptr && *OverlapCount > 0;
}

void APBCollisionBumperTriggerActor::AddBounceVelocityToBall(APBBallBase* Ball, const FHitResult& Hit) const
{
	if (!IsValid(Ball))
	{
		return;
	}

	const IStatProvider* StatProvider = PBInterfaceUtils::FindInterface<IStatProvider>(Ball);
	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball);
	if (!StatProvider || !Movable)
	{
		return;
	}

	const int32 BallBounce = StatProvider->GetStat(PBStatNames::Bounciness);
	const float BounceForce = BallBounce + BounceVelocityStrength;
	if (BounceForce <= 0.0f)
	{
		return;
	}

	FVector BounceDirection = Hit.ImpactNormal;
	BounceDirection.Z = 0.0f;
	if (!BounceDirection.Normalize())
	{
		return;
	}

	Movable->AddVelocity(BounceDirection * BounceForce);
}

void APBCollisionBumperTriggerActor::HandleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!CanReactToBall())
	{
		return;
	}

	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball) || !IsBallInTriggerArea(Ball))
	{
		return;
	}

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->PlayImpactReaction(Hit);
	}

	AddBounceVelocityToBall(Ball, Hit);
	IncreaseTrigger(Ball, Hit);
}

void APBCollisionBumperTriggerActor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	if (!CanReactToBall())
	{
		return;
	}

	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	int32& OverlapCount = TriggeringBallOverlapCounts.FindOrAdd(BallKey);
	++OverlapCount;
}

void APBCollisionBumperTriggerActor::HandleTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	int32* OverlapCount = TriggeringBallOverlapCounts.Find(BallKey);
	if (OverlapCount == nullptr)
	{
		return;
	}

	--(*OverlapCount);
	if (*OverlapCount > 0)
	{
		return;
	}

	TriggeringBallOverlapCounts.Remove(BallKey);
}
