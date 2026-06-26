// Fill out your copyright notice in the Description page of Project Settings.


#include "PBGateBumperTriggerActor.h"

#include "Components/PrimitiveComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"

APBGateBumperTriggerActor::APBGateBumperTriggerActor()
{
	TriggerType = EPBBumperTriggerType::PassCount;
}

void APBGateBumperTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	RegisterGateAreas();
}

void APBGateBumperTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UPrimitiveComponent* GateArea : GateAreas)
	{
		if (!IsValid(GateArea))
		{
			continue;
		}

		GateArea->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&APBGateBumperTriggerActor::HandleGateBeginOverlap);
		GateArea->OnComponentEndOverlap.RemoveDynamic(
			this,
			&APBGateBumperTriggerActor::HandleGateEndOverlap);
	}

	GateAreas.Reset();
	PassingBallOverlapCounts.Reset();
	PassingBallEntrySides.Reset();

	Super::EndPlay(EndPlayReason);
}

void APBGateBumperTriggerActor::RegisterGateAreas()
{
	// BP에서 만든 통과 판정 영역을 태그 기준으로 찾아 overlap 이벤트에 연결한다.
	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		GateAreaTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		SetupGateArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}
}

void APBGateBumperTriggerActor::SetupGateArea(UPrimitiveComponent* GateArea)
{
	if (!IsValid(GateArea) || GateAreas.Contains(GateArea))
	{
		return;
	}

	// 같은 컴포넌트가 중복 등록되어도 overlap이 한 번만 오도록 바인딩한다.
	GateArea->SetGenerateOverlapEvents(true);
	GateArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBGateBumperTriggerActor::HandleGateBeginOverlap);
	GateArea->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APBGateBumperTriggerActor::HandleGateEndOverlap);
	GateAreas.Add(GateArea);
}

float APBGateBumperTriggerActor::CalculateGateSide(
	const UPrimitiveComponent* GateArea,
	const APBBallBase* Ball) const
{
	if (!IsValid(GateArea) || !IsValid(Ball))
	{
		return 0.0f;
	}

	const FVector DirectionToBall = Ball->GetActorLocation() - GateArea->GetComponentLocation();
	return FVector::DotProduct(DirectionToBall, GateArea->GetForwardVector());
}

void APBGateBumperTriggerActor::HandleGateBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	if (!CanIncreaseTrigger())
	{
		return;
	}

	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	int32& OverlapCount = PassingBallOverlapCounts.FindOrAdd(BallKey);
	++OverlapCount;

	if (OverlapCount == 1)
	{
		// 첫 진입 시점의 면만 저장한다.
		PassingBallEntrySides.Add(BallKey, CalculateGateSide(OverlappedComponent, Ball));
	}
}

void APBGateBumperTriggerActor::HandleGateEndOverlap(
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
	int32* OverlapCount = PassingBallOverlapCounts.Find(BallKey);
	if (OverlapCount == nullptr)
	{
		return;
	}

	--(*OverlapCount);
	if (*OverlapCount > 0)
	{
		return;
	}

	PassingBallOverlapCounts.Remove(BallKey);

	float EntrySide = 0.0f;
	const bool IsEntrySideFound = PassingBallEntrySides.RemoveAndCopyValue(BallKey, EntrySide);
	if (!CanIncreaseTrigger() || !IsEntrySideFound)
	{
		return;
	}

	const float ExitSide = CalculateGateSide(OverlappedComponent, Ball);
	if (EntrySide * ExitSide >= 0.0f)
	{
		return;
	}

	IncreaseTrigger(Ball, FHitResult());
}
