// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCollisionBumper.h"

#include "Components/PrimitiveComponent.h"
#include "PinBallLike/Actor/Ball/BallBase.h"

APBCollisionBumper::APBCollisionBumper()
{
	PrimaryActorTick.bCanEverTick = false;

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

void APBCollisionBumper::AddBounceVelocityToBall(ABallBase* Ball, const FHitResult& Hit) const
{
	if (BounceVelocityStrength <= 0.0f || !IsValid(Ball))
	{
		return;
	}

	FVector BounceDirection = Hit.ImpactNormal;
	BounceDirection.Z = 0.0f;
	if (!BounceDirection.Normalize())
	{
		return;
	}

	Ball->AddVelocity(BounceDirection * BounceVelocityStrength);
}

void APBCollisionBumper::HandleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	ABallBase* Ball = Cast<ABallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}
	//TODO 콤보 누적 횟수 증가

	AddBounceVelocityToBall(Ball, Hit);
	OnCollisionBumperHit(Ball, Hit);
	AddTriggerCount(Ball);
}
