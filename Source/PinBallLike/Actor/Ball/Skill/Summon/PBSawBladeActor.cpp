#include "PBSawBladeActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"

APBSawBladeActor::APBSawBladeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(20.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("SkillMovement"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), nullptr);
	PhysicsComponent->SetGravityEnabled(false);
	PhysicsComponent->SetBounceDamping(1.0f);
	PhysicsComponent->OnBallMovementHit.AddUniqueDynamic(
		this,
		&APBSawBladeActor::HandleMovementHit);

	DamageComponent = CreateDefaultSubobject<UPBTimedAreaDamageComponent>(TEXT("DamageComponent"));
}

void APBSawBladeActor::InitializeEffect(
	APBBallBase* InOwnerBall,
	const int32 InDamageAmount,
	const int32 InGroggyAmount,
	const float InDuration,
	const int32 InMaxBounceCount)
{
	OwnerBall = InOwnerBall;
	DamageAmount = FMath::Max(InDamageAmount, 0);
	Duration = FMath::Max(InDuration, 0.0f);
	MaxBounceCount = FMath::Max(InMaxBounceCount, 1);

	const float SafeDamageInterval = FMath::Max(DamageInterval, 0.01f);
	const int32 DamageCount = FMath::Max(
		FMath::CeilToInt(Duration / SafeDamageInterval),
		1);
	DamageComponent->SetGroggyAmount(InGroggyAmount);
	DamageComponent->InitializeDamageArea(CollisionSphere.Get(), OwnerBall);
	DamageComponent->ConfigureDamage(DamageAmount, Duration, DamageCount);
	DamageComponent->OnAreaDamageApplied.AddUniqueDynamic(
		this,
		&APBSawBladeActor::HandleDamageApplied);
	DamageComponent->OnEffectFinished.AddUniqueDynamic(
		this,
		&APBSawBladeActor::HandleDamageFinished);
}

void APBSawBladeActor::ActivateEffect(FVector Direction)
{
	Direction.Z = 0.0f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero()
		|| !IsValid(OwnerBall)
		|| DamageAmount <= 0
		|| Duration <= 0.0f
		|| !DamageComponent->ActivateEffect())
	{
		FinishEffect();
		return;
	}

	PhysicsComponent->Launch(Direction, MovementSpeed);
	OnActivated();
}

void APBSawBladeActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DamageComponent->DeactivateEffect();
	Super::EndPlay(EndPlayReason);
}

void APBSawBladeActor::HandleMovementHit(const FHitResult&)
{
	if (bFinished)
	{
		return;
	}

	++CurrentBounceCount;
	OnBounce();

	if (CurrentBounceCount >= MaxBounceCount)
	{
		FinishEffect();
	}
}

void APBSawBladeActor::HandleDamageApplied(
	AActor* Target,
	const int32 AppliedDamage,
	const FVector HitLocation)
{
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBDamageLogMessage Message;
		Message.LogType = EPBDamageLogType::Skill;
		Message.DamageAmount = AppliedDamage;
		Message.HitLocation = HitLocation;
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_UI_DamageLog_Requested,
			Message);
	}
	OnHit(Target, AppliedDamage, HitLocation);
}

void APBSawBladeActor::HandleDamageFinished()
{
	FinishEffect();
}

void APBSawBladeActor::FinishEffect()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	DamageComponent->DeactivateEffect();
	PhysicsComponent->StopMovement();
	PhysicsComponent->PauseMovement();
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnFinished();
	OnEffectFinished.Broadcast(this);
	Destroy();
}
