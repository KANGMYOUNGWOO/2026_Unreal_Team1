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

	RegisterTaggedAreas();
}

void APBCollisionBumper::RegisterTaggedAreas()
{
	// BP에서 만든 물리 충돌 영역과 발동 트리거 영역을 태그 기준으로 자동 수집한다.
	TArray<UActorComponent*> TaggedCollisionComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		CollisionAreaTag);

	for (UActorComponent* TaggedComponent : TaggedCollisionComponents)
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

void APBCollisionBumper::SetupCollisionArea(UPrimitiveComponent* CollisionArea)
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

void APBCollisionBumper::SetupTriggerArea(UPrimitiveComponent* TriggerArea)
{
	if (!IsValid(TriggerArea) || TriggerAreas.Contains(TriggerArea))
	{
		return;
	}

	// 트리거 영역은 볼이 유효 발동 면 안에 있는지 기록하는 용도로만 사용한다.
	TriggerArea->SetGenerateOverlapEvents(true);
	TriggerArea->OnComponentBeginOverlap.AddUniqueDynamic(this, &APBCollisionBumper::HandleTriggerBeginOverlap);
	TriggerArea->OnComponentEndOverlap.AddUniqueDynamic(this, &APBCollisionBumper::HandleTriggerEndOverlap);
	TriggerAreas.Add(TriggerArea);
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

bool APBCollisionBumper::IsBallInTriggerArea(APBBallBase* Ball) const
{
	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	const int32* OverlapCount = TriggeringBallOverlapCounts.Find(BallKey);
	return OverlapCount != nullptr && *OverlapCount > 0;
}

void APBCollisionBumper::HandleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	IncreaseComboCount(OtherActor);
	
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball) || !IsBallInTriggerArea(Ball))
	{
		return;
	}

	IncreaseComboCount(OtherActor);
	AddBounceVelocityToBall(Ball, Hit);

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->PlayImpactReaction(Hit);
	}

	OnCollisionBumperHit(Ball, Hit);
	AddTriggerCount(Ball);
}

void APBCollisionBumper::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	int32& OverlapCount = TriggeringBallOverlapCounts.FindOrAdd(BallKey);
	++OverlapCount;
}

void APBCollisionBumper::HandleTriggerEndOverlap(
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
