#include "PBBossSnakeProjectilePattern.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossProjectile.h"
#include "PinBallLike/Actor/Boss/SnakeBoss.h"

UPBBossSnakeProjectilePattern::UPBBossSnakeProjectilePattern()
{
	ProjectileClass = APBBossProjectile::StaticClass();
}

bool UPBBossSnakeProjectilePattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss) && ProjectileClass && ProjectileCount > 0;
}

void UPBBossSnakeProjectilePattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	FiredProjectileCount = 0;
	CacheInitialBossRotation(Boss);

	if (!GetOwnerBoss() || !ProjectileClass || ProjectileCount <= 0)
	{
		FinishProjectilePattern();
		return;
	}

	ApplySnakeProjectilePose(Boss, 1.0f);

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

		if (FiredProjectileCount < ProjectileCount && GetOwnerBoss())
		{
			FinishPattern();
		}
		return;
	}

	FireProjectile();

	if (FiredProjectileCount < ProjectileCount)
	{
		Boss->GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&UPBBossSnakeProjectilePattern::FireProjectile,
			FireIntervalSeconds,
			true);
	}
}

void UPBBossSnakeProjectilePattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	ClearPatternTimers();
	ApplySnakeProjectilePose(Boss, 0.0f);
	RestoreInitialBossRotation(Boss);
	FiredProjectileCount = 0;
}

void UPBBossSnakeProjectilePattern::ApplySnakeProjectilePose(APBBossBase* Boss, float Alpha) const
{
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		SnakeBoss->SetSnakeProjectilePose(ClampedAlpha > 0.0f, ClampedAlpha);
	}
}

void UPBBossSnakeProjectilePattern::CacheInitialBossRotation(APBBossBase* Boss)
{
	IsInitialBossRotationCached = IsValid(Boss);
	if (IsInitialBossRotationCached)
	{
		InitialBossRotation = Boss->GetActorRotation();
	}
}

void UPBBossSnakeProjectilePattern::RestoreInitialBossRotation(APBBossBase* Boss)
{
	if (IsInitialBossRotationCached && IsValid(Boss))
	{
		Boss->SetActorRotation(InitialBossRotation);
	}

	IsInitialBossRotationCached = false;
}

void UPBBossSnakeProjectilePattern::FinishProjectilePattern()
{
	APBBossBase* Boss = GetOwnerBoss();
	ClearPatternTimers();
	ApplySnakeProjectilePose(Boss, 0.0f);
	RestoreInitialBossRotation(Boss);
	FinishPattern();
}

void UPBBossSnakeProjectilePattern::FireProjectile()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss || !ProjectileClass || FiredProjectileCount >= ProjectileCount)
	{
		FinishProjectilePattern();
		return;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		FinishProjectilePattern();
		return;
	}

	FVector SpawnLocation = GetProjectileSpawnLocation();
	FVector ProjectileDirection = GetProjectileDirection(SpawnLocation);
	FRotator SpawnRotation = ProjectileDirection.Rotation();

	APBBossProjectile* Projectile = World->SpawnActorDeferred<APBBossProjectile>(
		ProjectileClass,
		FTransform(SpawnRotation, SpawnLocation),
		Boss,
		Boss,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Projectile)
	{
		Projectile->SetProjectileDirection(ProjectileDirection);
		Projectile->SetProjectileSpeed(ProjectileSpeed);
		Projectile->SetSourcePatternName(PatternName.IsNone() ? GetClass()->GetFName() : PatternName);
		Projectile->SetDamageAmount(DamageAmount);
		Projectile->FinishSpawning(FTransform(SpawnRotation, SpawnLocation));
		PlayPatternSFX(Boss);
	}

	++FiredProjectileCount;

	UE_LOG(LogTemp, Warning, TEXT("Boss Projectile Fired: Boss=%s, Projectile=%s, Count=%d/%d"),
		*GetNameSafe(Boss),
		*GetNameSafe(Projectile),
		FiredProjectileCount,
		ProjectileCount);

	if (FiredProjectileCount >= ProjectileCount)
	{
		ClearPatternTimers();

		if (FireIntervalSeconds > 0.0f)
		{
			Boss->GetWorldTimerManager().SetTimer(
				FireTimerHandle,
				this,
				&UPBBossSnakeProjectilePattern::FinishProjectilePattern,
				FireIntervalSeconds,
				false);
			return;
		}

		FinishProjectilePattern();
	}
}

void UPBBossSnakeProjectilePattern::ClearFireTimer()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}
}

void UPBBossSnakeProjectilePattern::ClearPatternTimers()
{
	ClearFireTimer();
}

FVector UPBBossSnakeProjectilePattern::GetProjectileSpawnLocation() const
{
	const APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return FVector::ZeroVector;
	}

	return Boss->GetActorLocation() + Boss->GetActorTransform().TransformVectorNoScale(SpawnOffset);
}

FVector UPBBossSnakeProjectilePattern::GetProjectileDirection(const FVector& SpawnLocation) const
{
	AActor* PinballActor = FindPinballActor();
	if (!PinballActor)
	{
		return GetOwnerBoss() ? GetOwnerBoss()->GetActorForwardVector() : FVector::ForwardVector;
	}

	const FVector Direction = PinballActor->GetActorLocation() - SpawnLocation;
	if (Direction.IsNearlyZero())
	{
		return GetOwnerBoss() ? GetOwnerBoss()->GetActorForwardVector() : FVector::ForwardVector;
	}

	return Direction.GetSafeNormal();
}
