// Fill out your copyright notice in the Description page of Project Settings.


#include "PBRailBumperTriggerActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Spline/PBBumperSplineMeshActor.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBRailBumperTriggerActor::APBRailBumperTriggerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	TriggerType = EPBBumperTriggerType::RailEnter;
}

void APBRailBumperTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	ResolveRailSplineComponent();
	RegisterRailAreas();
}

void APBRailBumperTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UPrimitiveComponent* TriggerArea : TriggerAreas)
	{
		if (IsValid(TriggerArea))
		{
			TriggerArea->OnComponentBeginOverlap.RemoveDynamic(
				this,
				&APBRailBumperTriggerActor::HandleTriggerBeginOverlap);
		}
	}

	TriggerAreas.Reset();
	FinishRailMove();

	Super::EndPlay(EndPlayReason);
}

void APBRailBumperTriggerActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsRailMoving() || !IsValid(RailSplineComponent) || DeltaSeconds <= 0.0f)
	{
		return;
	}

	// 레일 이동 중인 공을 spline 거리 기준으로 이동시킴.
	const float SplineLength = RailSplineComponent->GetSplineLength();
	CurrentRailDistance = FMath::Min(CurrentRailDistance + RailMoveSpeed * DeltaSeconds, SplineLength);

	const FVector RailLocation = RailSplineComponent->GetLocationAtDistanceAlongSpline(
		CurrentRailDistance,
		ESplineCoordinateSpace::World);
	MovingBall->SetActorLocation(RailLocation);

	const float Alpha = SplineLength > 0.0f ? CurrentRailDistance / SplineLength : 1.0f;
	OnRailMoveUpdated(MovingBall, Alpha);

	if (CurrentRailDistance >= SplineLength)
	{
		FinishRailMove();
	}
}

void APBRailBumperTriggerActor::RegisterRailAreas()
{
	// BP에서 지정한 태그를 가진 overlap 영역을 레일 진입 트리거로 등록함.
	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		TriggerAreaTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		SetupTriggerArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}
}

void APBRailBumperTriggerActor::SetupTriggerArea(UPrimitiveComponent* TriggerArea)
{
	if (!IsValid(TriggerArea) || TriggerAreas.Contains(TriggerArea))
	{
		return;
	}

	TriggerArea->SetGenerateOverlapEvents(true);
	TriggerArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBRailBumperTriggerActor::HandleTriggerBeginOverlap);
	TriggerAreas.Add(TriggerArea);
}

void APBRailBumperTriggerActor::ResolveRailSplineComponent()
{
	// 맵에 배치된 spline mesh actor를 우선 사용하고, 없으면 내부 spline 태그를 검색함.
	if (IsValid(RailSplineActor))
	{
		RailSplineComponent = RailSplineActor->GetSplineComponent();
		if (IsValid(RailSplineComponent))
		{
			return;
		}
	}

	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		USplineComponent::StaticClass(),
		RailSplineTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		USplineComponent* SplineComponent = Cast<USplineComponent>(TaggedComponent);
		if (IsValid(SplineComponent))
		{
			RailSplineComponent = SplineComponent;
			return;
		}
	}
}

bool APBRailBumperTriggerActor::StartRailMove(APBBallBase* Ball)
{
	if (!IsValid(Ball) || !IsValid(RailSplineComponent) || IsRailMoving())
	{
		return false;
	}

	MovingBall = Ball;
	MovingBallMovable = PBInterfaceUtils::FindInterface<IMovable>(Ball);
	CurrentRailDistance = 0.0f;

	// 기존 공 이동을 멈추고 rail 이동이 직접 위치를 제어하도록 함.
	if (MovingBallMovable)
	{
		if (IsStopBallMovementOnStart)
		{
			MovingBallMovable->StopMovement();
		}
		if (IsPauseBallMovementOnStart)
		{
			MovingBallMovable->PauseMovement();
		}
	}

	MovingBall->SetActorLocation(RailSplineComponent->GetLocationAtDistanceAlongSpline(
		CurrentRailDistance,
		ESplineCoordinateSpace::World));

	OnRailMoveStarted(MovingBall);
	SetActorTickEnabled(true);
	return true;
}

void APBRailBumperTriggerActor::FinishRailMove()
{
	APBBallBase* FinishedBall = MovingBall;

	// rail 끝 방향으로 추가 속도를 부여한 뒤 공 이동을 재개함.
	AddExitVelocityToBall();

	if (MovingBallMovable && IsResumeBallMovementOnFinish)
	{
		MovingBallMovable->ResumeMovement();
	}

	MovingBall = nullptr;
	MovingBallMovable = nullptr;
	CurrentRailDistance = 0.0f;
	SetActorTickEnabled(false);

	if (IsValid(FinishedBall))
	{
		OnRailMoveFinished(FinishedBall);
	}
}

void APBRailBumperTriggerActor::AddExitVelocityToBall()
{
	if (!MovingBallMovable || !IsValid(RailSplineComponent) || ExitVelocityStrength <= 0.0f)
	{
		return;
	}

	// spline 끝 지점 tangent를 기준으로 rail 이탈 방향을 계산함.
	const float SplineLength = RailSplineComponent->GetSplineLength();
	FVector ExitDirection = RailSplineComponent->GetTangentAtDistanceAlongSpline(
		SplineLength,
		ESplineCoordinateSpace::World);
	ExitDirection.Z = 0.0f;

	if (!ExitDirection.Normalize())
	{
		return;
	}

	MovingBallMovable->AddVelocity(ExitDirection * ExitVelocityStrength);
}

bool APBRailBumperTriggerActor::IsRailMoving() const
{
	return IsValid(MovingBall);
}

void APBRailBumperTriggerActor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	//if (!CanReactToBall() || !CanIncreaseTrigger())
	//{
	//	return;
	//}

	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	if (StartRailMove(Ball))
	{
		IncreaseTrigger(Ball, SweepResult);
	}
}
