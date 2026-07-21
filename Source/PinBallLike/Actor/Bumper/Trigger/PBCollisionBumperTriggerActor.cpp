// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCollisionBumperTriggerActor.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "TimerManager.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperReactionComponent.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
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
	LastHitResponseTimes.Reset();

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

	if (CollisionAreas.IsEmpty() || TriggerAreas.IsEmpty())
	{
		const APBModularBumperBase* Bumper = GetOwnerBumper();
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Collision trigger setup incomplete. Trigger=%s Class=%s BumperRow=%s Position=%s CollisionTag=%s CollisionAreas=%d TriggerTag=%s TriggerAreas=%d"),
			*GetNameSafe(this),
			*GetPathNameSafe(GetClass()),
			IsValid(Bumper) ? *Bumper->GetBumperRowId().ToString() : TEXT("None"),
			*UEnum::GetValueAsString(GetPositionId()),
			*CollisionAreaTag.ToString(),
			CollisionAreas.Num(),
			*TriggerAreaTag.ToString(),
			TriggerAreas.Num());
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

bool APBCollisionBumperTriggerActor::IsBallInTriggerArea(AActor* BallActor) const
{
	const TWeakObjectPtr<AActor> BallKey = BallActor;
	const int32* OverlapCount = TriggeringBallOverlapCounts.Find(BallKey);
	return OverlapCount != nullptr && *OverlapCount > 0;
}

bool APBCollisionBumperTriggerActor::IsHitPointInsideTriggerArea(const FVector& HitPoint) const
{
	if (HitPoint.ContainsNaN())
	{
		return false;
	}

	for (const UPrimitiveComponent* TriggerArea : TriggerAreas)
	{
		if (!IsValid(TriggerArea))
		{
			continue;
		}

		if (const UBoxComponent* BoxArea = Cast<UBoxComponent>(TriggerArea))
		{
			const FTransform& AreaTransform = BoxArea->GetComponentTransform();
			const FVector LocalHitPoint = AreaTransform.InverseTransformPosition(HitPoint);
			const FVector BoxExtent = BoxArea->GetUnscaledBoxExtent();
			const FVector AreaScale = AreaTransform.GetScale3D().GetAbs();
			const FVector LocalTolerance(
				FMath::IsNearlyZero(AreaScale.X) ? 0.0f : TriggerAreaHitPointTolerance / AreaScale.X,
				FMath::IsNearlyZero(AreaScale.Y) ? 0.0f : TriggerAreaHitPointTolerance / AreaScale.Y,
				FMath::IsNearlyZero(AreaScale.Z) ? 0.0f : TriggerAreaHitPointTolerance / AreaScale.Z);

			if (FMath::Abs(LocalHitPoint.X) <= BoxExtent.X + LocalTolerance.X
				&& FMath::Abs(LocalHitPoint.Y) <= BoxExtent.Y + LocalTolerance.Y
				&& FMath::Abs(LocalHitPoint.Z) <= BoxExtent.Z + LocalTolerance.Z)
			{
				return true;
			}
			continue;
		}

		FVector ClosestPoint;
		const float Distance = TriggerArea->GetClosestPointOnCollision(HitPoint, ClosestPoint);
		if (Distance >= 0.0f && Distance <= TriggerAreaHitPointTolerance)
		{
			return true;
		}
	}

	return false;
}

bool APBCollisionBumperTriggerActor::TryBeginHitResponse(AActor* MovableActor)
{
	UWorld* World = GetWorld();
	if (!IsValid(MovableActor) || !IsValid(World))
	{
		return false;
	}

	const TWeakObjectPtr<AActor> MovableKey = MovableActor;
	const double CurrentTime = World->GetTimeSeconds();
	const double MinimumInterval = FMath::Max(MinimumHitResponseInterval, 0.0f);
	if (const double* LastResponseTime = LastHitResponseTimes.Find(MovableKey))
	{
		if (CurrentTime - *LastResponseTime < MinimumInterval)
		{
			return false;
		}
	}

	LastHitResponseTimes.Add(MovableKey, CurrentTime);
	return true;
}

bool APBCollisionBumperTriggerActor::TryResolveBounceDirection(
	const FVector& ImpactNormal,
	const FVector& MovableLocation,
	const FVector& ImpactPoint,
	const FVector& IncomingVelocity,
	FVector& OutBounceDirection)
{
	OutBounceDirection = FVector::ZeroVector;

	FVector CandidateDirection = ImpactNormal;
	CandidateDirection.Z = 0.0f;
	if (!CandidateDirection.Normalize())
	{
		return false;
	}

	FVector OutwardReference = MovableLocation - ImpactPoint;
	OutwardReference.Z = 0.0f;
	if (OutwardReference.Normalize())
	{
		if (FVector::DotProduct(CandidateDirection, OutwardReference) < 0.0f)
		{
			CandidateDirection *= -1.0f;
		}
	}
	else
	{
		FVector IncomingDirection = IncomingVelocity;
		IncomingDirection.Z = 0.0f;
		if (IncomingDirection.Normalize()
			&& FVector::DotProduct(CandidateDirection, IncomingDirection) > 0.0f)
		{
			CandidateDirection *= -1.0f;
		}
	}

	OutBounceDirection = CandidateDirection;
	return true;
}

bool APBCollisionBumperTriggerActor::QueueBounceVelocity(AActor* MovableActor, const FHitResult& Hit)
{
	if (!IsValid(MovableActor))
	{
		return false;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(MovableActor);
	if (!Movable)
	{
		return false;
	}

	int32 BallBounce = 0;
	if (const IStatProvider* StatProvider = PBInterfaceUtils::FindInterface<IStatProvider>(MovableActor))
	{
		BallBounce = StatProvider->GetStat(PBStatNames::Bounciness);
	}
	const float BounceForce = BallBounce + BounceVelocityStrength;
	if (BounceForce <= 0.0f)
	{
		return false;
	}

	FVector BounceDirection;
	if (!TryResolveBounceDirection(
		Hit.ImpactNormal,
		MovableActor->GetActorLocation(),
		Hit.ImpactPoint,
		Movable->GetVelocity(),
		BounceDirection))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	const TWeakObjectPtr<AActor> WeakMovableActor = MovableActor;
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(
		this,
		[WeakMovableActor, BounceDirection, BounceForce]()
		{
			AActor* ResolvedActor = WeakMovableActor.Get();
			if (!IsValid(ResolvedActor))
			{
				return;
			}

			if (IMovable* ResolvedMovable = PBInterfaceUtils::FindInterface<IMovable>(ResolvedActor))
			{
				ResolvedMovable->AddVelocity(BounceDirection * BounceForce);
			}
		}));
	return true;
}

void APBCollisionBumperTriggerActor::HandleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!CanReactToMovableActor())
	{
		return;
	}

	if (!PBInterfaceUtils::FindInterface<IMovable>(OtherActor))
	{
		return;
	}

	const bool bIsInValidTriggerArea = bUseHitPointTriggerAreaValidation
		? IsHitPointInsideTriggerArea(Hit.ImpactPoint)
		: IsBallInTriggerArea(OtherActor);
	if (!bIsInValidTriggerArea)
	{
		return;
	}
	if (!TryBeginHitResponse(OtherActor))
	{
		return;
	}

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->PlayImpactReaction(Hit);
	}

	if (QueueBounceVelocity(OtherActor, Hit))
	{
		PlayImpactCameraShake();
	}
	IncreaseTrigger(OtherActor, Hit);
}

void APBCollisionBumperTriggerActor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	if (!CanReactToMovableActor())
	{
		return;
	}

	if (!PBInterfaceUtils::FindInterface<IMovable>(OtherActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> BallKey = OtherActor;
	int32& OverlapCount = TriggeringBallOverlapCounts.FindOrAdd(BallKey);
	++OverlapCount;
}

void APBCollisionBumperTriggerActor::HandleTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> BallKey = OtherActor;
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
	LastHitResponseTimes.Remove(BallKey);
}
