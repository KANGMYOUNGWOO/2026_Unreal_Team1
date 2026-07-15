#include "PBBossProjectile.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBBossProjectile::APBBossProjectile()
{
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

void APBBossProjectile::SetSourcePatternName(FName NewSourcePatternName)
{
	SourcePatternName = NewSourcePatternName;
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
	ActivateProjectile();
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

	IDamageable* Damageable = Cast<IDamageable>(Pinball);
	if (!Damageable)
	{
		Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Pinball);
	}

	if (Damageable && !Damageable->IsDead())
	{
		Damageable->TakeDamage(DamageAmount);
		UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
			*SourcePatternName.ToString(),
			DamageAmount,
			*GetNameSafe(Pinball));
	}

	UE_LOG(LogTemp, Warning, TEXT("Boss Projectile Hit Pinball: Projectile=%s, Pinball=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Pinball));
}
