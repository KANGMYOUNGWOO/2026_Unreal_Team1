#include "PBBossProjectilePattern.h"

#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossProjectile.h"

UPBBossProjectilePattern::UPBBossProjectilePattern()
{
	ProjectileClass = APBBossProjectile::StaticClass();
}

void UPBBossProjectilePattern::StartPattern_Implementation(APBBossBase* Boss)
{
	if (!Boss || !ProjectileClass || ProjectileCount <= 0)
	{
		FinishPattern();
		return;
	}

	OwnerBoss = Boss;
	FiredProjectileCount = 0;

	if (FireIntervalSeconds <= 0.0f)
	{
		while (FiredProjectileCount < ProjectileCount)
		{
			const int32 PreviousFiredProjectileCount = FiredProjectileCount;
			FireProjectile();

			if (FiredProjectileCount == PreviousFiredProjectileCount)
			{
				break;
			}
		}

		return;
	}

	FireProjectile();

	if (FiredProjectileCount < ProjectileCount)
	{
		Boss->GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&UPBBossProjectilePattern::FireProjectile,
			FireIntervalSeconds,
			true);
	}
}

void UPBBossProjectilePattern::CancelPattern_Implementation(APBBossBase* Boss)
{
	ClearFireTimer();
	OwnerBoss = nullptr;
	FiredProjectileCount = 0;
}

void UPBBossProjectilePattern::FireProjectile()
{
	if (!OwnerBoss || !ProjectileClass || FiredProjectileCount >= ProjectileCount)
	{
		ClearFireTimer();
		FinishPattern();
		return;
	}

	UWorld* World = OwnerBoss->GetWorld();
	if (!World)
	{
		ClearFireTimer();
		FinishPattern();
		return;
	}

	const FVector SpawnLocation = GetProjectileSpawnLocation();
	const FRotator SpawnRotation = GetProjectileSpawnRotation(SpawnLocation);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerBoss;
	SpawnParameters.Instigator = OwnerBoss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APBBossProjectile* Projectile = World->SpawnActor<APBBossProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (Projectile)
	{
		Projectile->SetProjectileSpeed(ProjectileSpeed);
	}

	++FiredProjectileCount;

	UE_LOG(LogTemp, Warning, TEXT("Boss Projectile Fired: Boss=%s, Projectile=%s, Count=%d/%d"),
		*GetNameSafe(OwnerBoss),
		*GetNameSafe(Projectile),
		FiredProjectileCount,
		ProjectileCount);

	if (FiredProjectileCount >= ProjectileCount)
	{
		ClearFireTimer();
		FinishPattern();
	}
}

void UPBBossProjectilePattern::ClearFireTimer()
{
	if (OwnerBoss)
	{
		OwnerBoss->GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}
}

FVector UPBBossProjectilePattern::GetProjectileSpawnLocation() const
{
	if (!OwnerBoss)
	{
		return FVector::ZeroVector;
	}

	return OwnerBoss->GetActorLocation() + OwnerBoss->GetActorTransform().TransformVectorNoScale(SpawnOffset);
}

FRotator UPBBossProjectilePattern::GetProjectileSpawnRotation(const FVector& SpawnLocation) const
{
	AActor* PinballActor = FindPinballActor();
	if (!PinballActor)
	{
		return OwnerBoss ? OwnerBoss->GetActorRotation() : FRotator::ZeroRotator;
	}

	const FVector Direction = PinballActor->GetActorLocation() - SpawnLocation;
	if (Direction.IsNearlyZero())
	{
		return OwnerBoss ? OwnerBoss->GetActorRotation() : FRotator::ZeroRotator;
	}

	return Direction.Rotation();
}

AActor* UPBBossProjectilePattern::FindPinballActor() const
{
	if (!OwnerBoss)
	{
		return nullptr;
	}

	UWorld* World = OwnerBoss->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return UGameplayStatics::GetActorOfClass(World, APBBallBase::StaticClass());
}
