#include "PBCircularBladeActor.h"

#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

APBCircularBladeActor::APBCircularBladeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
	AttackSphere->SetupAttachment(Root);
	AttackSphere->InitSphereRadius(100.0f);
	AttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	AttackSphere->SetGenerateOverlapEvents(true);

	TimedAreaDamageComponent = CreateDefaultSubobject<UPBTimedAreaDamageComponent>(TEXT("TimedAreaDamageComponent"));
}
void APBCircularBladeActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	if (TimedAreaDamageComponent)
	{
		BindDamageEvents(TimedAreaDamageComponent);
		TimedAreaDamageComponent->InitializeDamageArea(
			AttackSphere,
			OwnerBall);
		TimedAreaDamageComponent->ConfigureDamage(
			GetSkillDamageAmount(),
			InSkillData.Duration,
			InSkillData.Value);
	}
}

void APBCircularBladeActor::EnterActiveState()
{
	if (!TimedAreaDamageComponent || !TimedAreaDamageComponent->ActivateEffect())
	{
		StopSkill();
		return;
	}

	BallPhysicsComponent = OwnerBall->FindComponentByClass<UPBBallPhysicsComponent>();
	TargetActor = FindTarget();
	ApplyTargetAcceleration();
	Super::EnterActiveState();
}

void APBCircularBladeActor::EnterFinishingState()
{
	DeactivateBlade();
	Super::EnterFinishingState();
}

void APBCircularBladeActor::EnterStoppingState()
{
	DeactivateBlade();
	Super::EnterStoppingState();
}

void APBCircularBladeActor::DeactivateBlade()
{
	if (TimedAreaDamageComponent)
	{
		TimedAreaDamageComponent->DeactivateEffect();
	}

	TargetActor.Reset();
	BallPhysicsComponent.Reset();
}

void APBCircularBladeActor::ApplyTargetAcceleration()
{
	const AActor* Target = TargetActor.Get();
	const FVector Direction = (Target->GetActorLocation() - OwnerBall->GetActorLocation()).GetSafeNormal2D();
	BallPhysicsComponent->AddVelocity(Direction * TargetAcceleration);
}
