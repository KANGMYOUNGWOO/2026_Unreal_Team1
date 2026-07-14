#include "PBLaserSkillActor.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

APBLaserSkillActor::APBLaserSkillActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	LaserCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LaserCollision"));
	LaserCollision->SetupAttachment(Root);
	LaserCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LaserCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	LaserCollision->SetGenerateOverlapEvents(true);

	TimedAreaDamageComponent = CreateDefaultSubobject<UPBTimedAreaDamageComponent>(TEXT("TimedAreaDamageComponent"));
}

void APBLaserSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const int32 InDamageAmount,
	const float InDuration,
	const int32 InDamageCount)
{
	Super::InitializeSkill(InOwnerBall, InDamageAmount, InDuration, InDamageCount);

	BindDamageEvents(TimedAreaDamageComponent);
	TimedAreaDamageComponent->InitializeDamageArea(LaserCollision, OwnerBall);
	TimedAreaDamageComponent->ConfigureDamage(
		InDamageAmount,
		InDuration,
		InDamageCount);
}

void APBLaserSkillActor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTargetTracking();
}

void APBLaserSkillActor::EnterActiveState()
{
	TargetActor = FindTarget();
	SetActorTickEnabled(true);
	UpdateTargetTracking();
	if (!TimedAreaDamageComponent->ActivateEffect())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBLaserSkillActor::EnterFinishingState()
{
	DeactivateLaser();
	Super::EnterFinishingState();
}

void APBLaserSkillActor::EnterStoppingState()
{
	DeactivateLaser();
	Super::EnterStoppingState();
}

void APBLaserSkillActor::DeactivateLaser()
{
	TimedAreaDamageComponent->DeactivateEffect();
	SetActorTickEnabled(false);
	TargetActor.Reset();
}

void APBLaserSkillActor::UpdateTargetTracking()
{
	if (!IsTargetValid(TargetActor.Get()))
	{
		TargetActor = FindTarget();
	}

	const AActor* Target = TargetActor.Get();
	if (!Target || !OwnerBall)
	{
		return;
	}

	const FVector Direction =
		(Target->GetActorLocation() - OwnerBall->GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}
