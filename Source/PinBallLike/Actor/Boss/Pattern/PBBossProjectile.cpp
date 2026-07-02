#include "PBBossProjectile.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"

APBBossProjectile::APBBossProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(15.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionSphere;
	ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
	ProjectileMovementComponent->MaxSpeed = ProjectileSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bSweepCollision = false;
}

void APBBossProjectile::SetProjectileSpeed(float NewProjectileSpeed)
{
	ProjectileSpeed = FMath::Max(0.0f, NewProjectileSpeed);
	ApplyProjectileSpeed();
}

float APBBossProjectile::GetProjectileSpeed() const
{
	return ProjectileSpeed;
}

void APBBossProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetProjectileCollision();

	if (CollisionSphere)
	{
		CollisionSphere->IgnoreActorWhenMoving(GetOwner(), true);
		CollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &APBBossProjectile::HandleProjectileBeginOverlap);
	}

	CheckInitialOverlappingPinballs();
	ApplyProjectileSpeed();
	SetLifeSpan(FMath::Max(0.1f, LifeTimeSeconds));
}

void APBBossProjectile::HandleProjectileBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	ProcessPinballOverlap(Cast<APBBallBase>(OtherActor));
}

void APBBossProjectile::ApplyProjectileSpeed()
{
	if (!ProjectileMovementComponent)
	{
		return;
	}

	ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
	ProjectileMovementComponent->MaxSpeed = ProjectileSpeed;
	ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileSpeed;
}

void APBBossProjectile::SetProjectileCollision()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
		PrimitiveComponent->SetGenerateOverlapEvents(true);
	}
}

void APBBossProjectile::CheckInitialOverlappingPinballs()
{
	if (!CollisionSphere)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	CollisionSphere->GetOverlappingActors(OverlappingActors, APBBallBase::StaticClass());

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!OverlappingActor || OverlappingActor == GetOwner())
		{
			continue;
		}

		ProcessPinballOverlap(Cast<APBBallBase>(OverlappingActor));
	}
}

void APBBossProjectile::ProcessPinballOverlap(APBBallBase* Pinball)
{
	if (!Pinball)
	{
		return;
	}

	const TObjectKey<APBBallBase> PinballKey(Pinball);
	if (ProcessedPinballs.Contains(PinballKey))
	{
		return;
	}

	ProcessedPinballs.Add(PinballKey);

	UE_LOG(LogTemp, Warning, TEXT("Boss Projectile Hit Pinball: Projectile=%s, Pinball=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Pinball));
}
