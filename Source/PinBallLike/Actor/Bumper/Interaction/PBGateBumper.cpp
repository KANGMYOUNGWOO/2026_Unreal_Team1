// Fill out your copyright notice in the Description page of Project Settings.


#include "PBGateBumper.h"

#include "Components/PrimitiveComponent.h"
#include "PinBallLike/Actor/Ball/BallBase.h"

APBGateBumper::APBGateBumper()
{
	PrimaryActorTick.bCanEverTick = false;

	BumperData.BumperType = EPBBumperType::Gate;
	BumperData.TriggerType = EPBBumperTriggerType::PassCount;
	BumperData.ActivationType = EPBBumperActivationType::Support;
}

void APBGateBumper::BeginPlay()
{
	Super::BeginPlay();

	RegisterTaggedGateAreas();
}

void APBGateBumper::RegisterTaggedGateAreas()
{
	// BP에서 게이트 모양을 자유롭게 만들고, Component Tag로 통과 판정 영역만 수집한다.
	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		GateAreaTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		RegisterGateArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}
}

void APBGateBumper::RegisterGateArea(UPrimitiveComponent* GateArea)
{
	if (!IsValid(GateArea) || GateAreas.Contains(GateArea))
	{
		return;
	}

	// 같은 컴포넌트가 여러 번 등록되어도 Overlap 이벤트가 중복 호출되지 않게 한다.
	GateArea->SetGenerateOverlapEvents(true);
	GateArea->OnComponentBeginOverlap.AddUniqueDynamic(this, &APBGateBumper::HandleGateBeginOverlap);
	GateArea->OnComponentEndOverlap.AddUniqueDynamic(this, &APBGateBumper::HandleGateEndOverlap);
	GateAreas.Add(GateArea);
}

// 볼이 게이트 기준 어느 면에 있는지 계산한다.
float APBGateBumper::CalculateGateSide(const UPrimitiveComponent* GateArea, const ABallBase* Ball) const
{
	if (!IsValid(GateArea) || !IsValid(Ball))
	{
		return 0.0f;
	}

	const FVector DirectionToBall = Ball->GetActorLocation() - GateArea->GetComponentLocation();
	return FVector::DotProduct(DirectionToBall, GateArea->GetForwardVector());
}

void APBGateBumper::HandleGateBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	ABallBase* Ball = Cast<ABallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<ABallBase> BallKey = Ball;
	int32& OverlapCount = PassingBallOverlapCounts.FindOrAdd(BallKey);
	++OverlapCount;

	if (OverlapCount == 1)
	{
		// 첫 진입 시점의 면만 저장한다.
		PassingBallEntrySides.Add(BallKey, CalculateGateSide(OverlappedComponent, Ball));
	}
}

void APBGateBumper::HandleGateEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ABallBase* Ball = Cast<ABallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<ABallBase> BallKey = Ball;
	int32* OverlapCount = PassingBallOverlapCounts.Find(BallKey);
	if (OverlapCount == nullptr)
	{
		return;
	}

	--(*OverlapCount);
	// 여러개의 게이트 판정용
	//if (*OverlapCount > 0)
	//{
	//	// 아직 다른 게이트 영역과 겹쳐 있으면 통과 판정을 미룬다.
	//	return;
	//}

	PassingBallOverlapCounts.Remove(BallKey);

	float EntrySide = 0.0f;
	const bool IsEntrySideFound = PassingBallEntrySides.RemoveAndCopyValue(BallKey, EntrySide);

	if (!IsEntrySideFound)
	{
		return;
	}

	const float ExitSide = CalculateGateSide(OverlappedComponent, Ball);
	if (EntrySide * ExitSide >= 0.0f)
	{
		// 들어온 면과 나간 면이 같으면 통과로 보지 않는다.
		return;
	}

	OnGateBumperPassed(Ball);
	AddTriggerCount(Ball);
}
