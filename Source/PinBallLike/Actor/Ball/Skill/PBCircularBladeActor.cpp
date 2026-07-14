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
void APBCircularBladeActor::InitializeBlade(
	APBBallBase* InOwnerBall,
	int32 InDamageAmount,
	float InDuration,
	int32 InDamageCount)
{
	OwnerBall = InOwnerBall;

	if (OwnerBall)
	{
		AttachToActor(OwnerBall, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		SetActorRelativeScale3D(FVector::OneVector);
		SetActorLocation(OwnerBall->GetActorLocation());
	}

	if (TimedAreaDamageComponent)
	{
		BindDamageEvents(TimedAreaDamageComponent);
		TimedAreaDamageComponent->InitializeDamageArea(
			AttackSphere,
			OwnerBall);
		TimedAreaDamageComponent->ConfigureDamage(
			InDamageAmount,
			InDuration,
			InDamageCount);
	}

	ActivateEffect();
}

bool APBCircularBladeActor::ActivateEffectInternal()
{
	if (!TimedAreaDamageComponent || !TimedAreaDamageComponent->ActivateEffect())
	{
		return false;
	}

	BallPhysicsComponent = OwnerBall->FindComponentByClass<UPBBallPhysicsComponent>();
	TargetActor = FindTarget();
	ApplyTargetAcceleration();
	return true;
}

void APBCircularBladeActor::DeactivateEffectInternal()
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
