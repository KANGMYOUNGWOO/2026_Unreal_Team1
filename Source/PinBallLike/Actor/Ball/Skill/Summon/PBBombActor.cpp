#include "PBBombActor.h"

#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBInstantDamageComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "TimerManager.h"

APBBombActor::APBBombActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(25.0f);
	CollisionSphere->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetEnableGravity(false);

	ExplosionArea = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionArea"));
	ExplosionArea->SetupAttachment(CollisionSphere);
	ExplosionArea->InitSphereRadius(200.0f);
	ExplosionArea->SetCollisionProfileName(TEXT("Skill"));
	ExplosionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExplosionArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBBombActor::HandleExplosionAreaBeginOverlap);

	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), nullptr);

	DamageComponent = CreateDefaultSubobject<UPBInstantDamageComponent>(TEXT("DamageComponent"));
}

void APBBombActor::InitializeBomb(
	APBBallBase* InOwnerBall,
	const int32 InDamageAmount,
	const int32 InGroggyAmount)
{
	OwnerBall = InOwnerBall;
	DamageAmount = FMath::Max(InDamageAmount, 0);
	DamageComponent->SetGroggyAmount(InGroggyAmount);
}

void APBBombActor::ActivateBomb(const FVector InitialVelocity)
{
	PhysicsComponent->SetVelocity(InitialVelocity);

	const float EarlyExplosionDelay = FMath::Max(
		ExplosionTime - EarlyExplosionRemainingTime,
		0.0f);
	if (EarlyExplosionDelay <= 0.0f)
	{
		EnableEarlyExplosion();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			EarlyExplosionTimerHandle,
			this,
			&APBBombActor::EnableEarlyExplosion,
			EarlyExplosionDelay,
			false);
	}

	GetWorldTimerManager().SetTimer(
		ExplosionTimerHandle,
		this,
		&APBBombActor::BeginExplosion,
		ExplosionTime,
		false);
}

void APBBombActor::Explode()
{
	if (!bExplosionRequested || bExploded)
	{
		return;
	}

	bExploded = true;
	ApplyExplosionDamage();
	OnExploded();
	OnBombFinished.Broadcast(this);
	Destroy();
}

void APBBombActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearExplosionTimers();
	Super::EndPlay(EndPlayReason);
}

void APBBombActor::HandleExplosionAreaBeginOverlap(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32,
	bool,
	const FHitResult&)
{
	if (bCanEarlyExplode
		&& IsValid(OtherActor)
		&& OtherActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		BeginExplosion();
	}
}

void APBBombActor::EnableEarlyExplosion()
{
	bCanEarlyExplode = true;
	if (IsBossInExplosionArea())
	{
		BeginExplosion();
	}
}

void APBBombActor::BeginExplosion()
{
	if (bExplosionRequested || bExploded)
	{
		return;
	}

	bExplosionRequested = true;
	ClearExplosionTimers();
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PhysicsComponent->SetGravityEnabled(false);
	PhysicsComponent->StopMovementSmoothly(StopDuration);
	RequestExplode();
}

void APBBombActor::ApplyExplosionDamage()
{
	TArray<AActor*> OverlappingActors;
	ExplosionArea->GetOverlappingActors(OverlappingActors);

	for (AActor* Target : OverlappingActors)
	{
		if (!IsValid(Target)
			|| !Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass())
			|| !DamageComponent->ApplyDamage(Target, DamageAmount))
		{
			continue;
		}

		FVector HitLocation = ExplosionArea->GetComponentLocation();
		FVector ClosestPoint;
		if (ExplosionArea->GetClosestPointOnCollision(
			Target->GetActorLocation(),
			ClosestPoint) >= 0.0f)
		{
			HitLocation = ClosestPoint;
		}

		if (UGameplayMessageSubsystem::HasInstance(this))
		{
			FPBDamageLogMessage Message;
			Message.Style = EPBDamageLogStyle::PlayerSkill;
			Message.DamageAmount = DamageAmount;
			Message.HitLocation = HitLocation;
			UGameplayMessageSubsystem::Get(this).BroadcastMessage(
				GameplayTags::Event_UI_DamageLog_Requested,
				Message);
		}

		OnHit(Target, DamageAmount, HitLocation);
	}
}

void APBBombActor::ClearExplosionTimers()
{
	GetWorldTimerManager().ClearTimer(EarlyExplosionTimerHandle);
	GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);
}

bool APBBombActor::IsBossInExplosionArea() const
{
	TArray<AActor*> OverlappingActors;
	ExplosionArea->GetOverlappingActors(OverlappingActors);

	for (const AActor* Target : OverlappingActors)
	{
		if (IsValid(Target)
			&& Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
		{
			return true;
		}
	}

	return false;
}
