// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCollisionBumper.h"

#include "Components/PrimitiveComponent.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperReactionComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBCollisionBumper::APBCollisionBumper()
{
	PrimaryActorTick.bCanEverTick = false;

	ReactionComponent = CreateDefaultSubobject<UPBBumperReactionComponent>(TEXT("ReactionComponent"));

	BumperData.TriggerType = EPBBumperTriggerType::HitCount;
}

void APBCollisionBumper::BeginPlay()
{
	Super::BeginPlay();

	RegisterTaggedCollisionAreas();
}

void APBCollisionBumper::RegisterTaggedCollisionAreas()
{
	// 범퍼마다 다른 콜리전 모양을 BP에서 구성하고, Component Tag로 판정 영역만 수집한다.
	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		CollisionAreaTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		RegisterCollisionArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}
}

void APBCollisionBumper::RegisterCollisionArea(UPrimitiveComponent* CollisionArea)
{
	if (!IsValid(CollisionArea) || CollisionAreas.Contains(CollisionArea))
	{
		return;
	}

	// 같은 컴포넌트가 여러 번 등록되어도 Hit 이벤트가 중복 호출되지 않게 한다.
	CollisionArea->SetNotifyRigidBodyCollision(true);
	CollisionArea->OnComponentHit.AddUniqueDynamic(this, &APBCollisionBumper::HandleComponentHit);
	CollisionAreas.Add(CollisionArea);
}

void APBCollisionBumper::AddBounceVelocityToBall(APBBallBase* Ball, const FHitResult& Hit) const
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

	const int32 ballBounce = StatProvider->GetStat(PBStatNames::Bounciness);
	int32 bounceForce = ballBounce + BounceVelocityStrength;

	if (bounceForce <= 0)
	{
		return;
	}
	
	FVector BounceDirection = Hit.ImpactNormal;
	BounceDirection.Z = 0.0f;
	if (!BounceDirection.Normalize())
	{
		return;
	}

	Movable->AddVelocity(BounceDirection * bounceForce);
}

void APBCollisionBumper::HandleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	IncreaseTriggerCount(OtherActor);
	
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}
	
	AddBounceVelocityToBall(Ball, Hit);

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->PlayImpactReaction(Hit);
	}

	OnCollisionBumperHit(Ball, Hit);
	AddTriggerCount(Ball);
}
