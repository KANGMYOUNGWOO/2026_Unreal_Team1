#include "PBBoomerangActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBInstantDamageComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "TimerManager.h"

APBBoomerangActor::APBBoomerangActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(20.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("SkillMovement"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBBoomerangActor::HandleDamageBeginOverlap);
	CollisionSphere->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APBBoomerangActor::HandleDamageEndOverlap);

	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), nullptr);
	PhysicsComponent->SetGravityEnabled(false);
	PhysicsComponent->SetBounceDamping(1.0f);
	PhysicsComponent->OnBallMovementHit.AddUniqueDynamic(
		this,
		&APBBoomerangActor::HandleMovementHit);

	DamageComponent = CreateDefaultSubobject<UPBInstantDamageComponent>(TEXT("DamageComponent"));
}

void APBBoomerangActor::InitializeEffect(
	APBBallBase* InOwnerBall,
	const int32 InDamageAmount,
	const int32 InGroggyAmount,
	const float InMaxPhaseDuration,
	const int32 InMaxBounceCount)
{
	OwnerBall = InOwnerBall;
	DamageAmount = FMath::Max(InDamageAmount, 0);
	DamageComponent->SetGroggyAmount(InGroggyAmount);
	MaxPhaseDuration = FMath::Max(InMaxPhaseDuration, 0.01f);
	MaxBounceCount = FMath::Max(InMaxBounceCount, 1);
}

void APBBoomerangActor::ActivateEffect(FVector Direction)
{
	Direction.Z = 0.0f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero() || !IsValid(OwnerBall) || DamageAmount <= 0)
	{
		FinishEffect();
		return;
	}

	PhysicsComponent->Launch(Direction, MovementSpeed);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&APBBoomerangActor::HandlePhaseTimeout,
		MaxPhaseDuration,
		false);
	OnActivated();
}

void APBBoomerangActor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bReturning)
	{
		return;
	}

	if (!IsValid(OwnerBall))
	{
		FinishEffect();
		return;
	}

	const FVector ToOwner = OwnerBall->GetActorLocation() - GetActorLocation();
	if (ToOwner.SizeSquared2D() <= FMath::Square(ReturnDistance))
	{
		FinishEffect();
		return;
	}

	PhysicsComponent->SetVelocity(ToOwner.GetSafeNormal2D() * MovementSpeed);
}

void APBBoomerangActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPhaseTimer();
	Super::EndPlay(EndPlayReason);
}

void APBBoomerangActor::HandleMovementHit(const FHitResult&)
{
	if (bReturning || bFinished)
	{
		return;
	}

	++CurrentBounceCount;
	OnBounce(CurrentBounceCount);
	if (CurrentBounceCount >= MaxBounceCount)
	{
		BeginReturn();
	}
}

void APBBoomerangActor::HandleDamageBeginOverlap(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32,
	bool,
	const FHitResult&)
{
	ApplyDamageToTarget(OtherActor);
}

void APBBoomerangActor::HandleDamageEndOverlap(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32)
{
	if (IsValid(OtherActor) && !CollisionSphere->IsOverlappingActor(OtherActor))
	{
		OverlappingTargets.Remove(TWeakObjectPtr<AActor>(OtherActor));
	}
}

void APBBoomerangActor::BeginReturn()
{
	if (bReturning || bFinished)
	{
		return;
	}

	bReturning = true;
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	SetActorTickEnabled(true);
	ClearPhaseTimer();
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&APBBoomerangActor::HandlePhaseTimeout,
		MaxPhaseDuration,
		false);
	OnReturning();
}

void APBBoomerangActor::HandlePhaseTimeout()
{
	if (bReturning)
	{
		FinishEffect();
	}
	else
	{
		BeginReturn();
	}
}

void APBBoomerangActor::ApplyDamageToTarget(AActor* Target)
{
	const TWeakObjectPtr<AActor> TargetKey(Target);
	if (!IsValid(Target)
		|| !Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass())
		|| OverlappingTargets.Contains(TargetKey)
		|| !DamageComponent->ApplyDamage(Target, DamageAmount))
	{
		return;
	}

	OverlappingTargets.Add(TargetKey);
	FVector HitLocation = CollisionSphere->GetComponentLocation();
	FVector ClosestPoint;
	if (CollisionSphere->GetClosestPointOnCollision(Target->GetActorLocation(), ClosestPoint) >= 0.0f)
	{
		HitLocation = ClosestPoint;
	}
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBDamageLogMessage Message;
		Message.LogType = EPBDamageLogType::Skill;
		Message.DamageAmount = DamageAmount;
		Message.HitLocation = HitLocation;
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_UI_DamageLog_Requested,
			Message);
	}
	OnHit(Target, DamageAmount, HitLocation);
}

void APBBoomerangActor::FinishEffect()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	ClearPhaseTimer();
	SetActorTickEnabled(false);
	PhysicsComponent->StopMovement();
	PhysicsComponent->PauseMovement();
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnFinished();
	OnEffectFinished.Broadcast(this);
	Destroy();
}

void APBBoomerangActor::ClearPhaseTimer()
{
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
}
