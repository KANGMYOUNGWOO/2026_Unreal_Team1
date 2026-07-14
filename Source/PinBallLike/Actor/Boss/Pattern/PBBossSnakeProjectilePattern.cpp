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

	if (!GetOwnerBoss() || !ProjectileClass || ProjectileCount <= 0)
	{
		FinishPattern();
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

void UPBBossSnakeProjectilePattern::FinishProjectilePattern()
{
	APBBossBase* Boss = GetOwnerBoss();
	ClearPatternTimers();
	ApplySnakeProjectilePose(Boss, 0.0f);
	FinishPattern();
}

void UPBBossSnakeProjectilePattern::FireProjectile()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss || !ProjectileClass || FiredProjectileCount >= ProjectileCount)
	{
		ClearPatternTimers();
		ApplySnakeProjectilePose(Boss, 0.0f);
		FinishPattern();
		return;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		ClearPatternTimers();
		ApplySnakeProjectilePose(Boss, 0.0f);
		FinishPattern();
		return;
	}

	FVector SpawnLocation = GetProjectileSpawnLocation();
	FRotator SpawnRotation = GetProjectileSpawnRotation(SpawnLocation);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Boss;
	SpawnParameters.Instigator = Boss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APBBossProjectile* Projectile = World->SpawnActor<APBBossProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (Projectile)
	{
		Projectile->SetProjectileSpeed(ProjectileSpeed);
		Projectile->SetSourcePatternName(PatternName.IsNone() ? GetClass()->GetFName() : PatternName);
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

FRotator UPBBossSnakeProjectilePattern::GetProjectileSpawnRotation(const FVector& SpawnLocation) const
{
	AActor* PinballActor = FindPinballActor();
	if (!PinballActor)
	{
		return GetOwnerBoss() ? GetOwnerBoss()->GetActorRotation() : FRotator::ZeroRotator;
	}

	const FVector Direction = PinballActor->GetActorLocation() - SpawnLocation;
	if (Direction.IsNearlyZero())
	{
		return GetOwnerBoss() ? GetOwnerBoss()->GetActorRotation() : FRotator::ZeroRotator;
	}

	return Direction.Rotation();
}
