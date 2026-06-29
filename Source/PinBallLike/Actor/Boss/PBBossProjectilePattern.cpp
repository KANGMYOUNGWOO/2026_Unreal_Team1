#include "PBBossProjectilePattern.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossProjectile.h"
#include "PinBallLike/Actor/Boss/SnakeBoss.h"

UPBBossProjectilePattern::UPBBossProjectilePattern()
{
	ProjectileClass = APBBossProjectile::StaticClass();
}

bool UPBBossProjectilePattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss) && ProjectileClass && ProjectileCount > 0;
}

void UPBBossProjectilePattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	FiredProjectileCount = 0;

	if (!GetOwnerBoss() || !ProjectileClass || ProjectileCount <= 0)
	{
		FinishPattern();
		return;
	}

	UpdateProjectileAim();
	if (Cast<ASnakeBoss>(Boss))
	{
		Boss->GetWorldTimerManager().SetTimer(
			AimTimerHandle,
			this,
			&UPBBossProjectilePattern::UpdateProjectileAim,
			AimUpdateIntervalSeconds,
			true);
	}

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

void UPBBossProjectilePattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	ClearPatternTimers();
	FiredProjectileCount = 0;
}

void UPBBossProjectilePattern::FireProjectile()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss || !ProjectileClass || FiredProjectileCount >= ProjectileCount)
	{
		ClearPatternTimers();
		FinishPattern();
		return;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		ClearPatternTimers();
		FinishPattern();
		return;
	}

	FVector SpawnLocation = GetProjectileSpawnLocation();
	FRotator SpawnRotation = GetProjectileSpawnRotation(SpawnLocation);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->FaceHeadDirection(SpawnRotation.Vector());
		SpawnLocation = GetProjectileSpawnLocation();
		SpawnRotation = GetProjectileSpawnRotation(SpawnLocation);
	}

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
		FinishPattern();
	}
}

void UPBBossProjectilePattern::UpdateProjectileAim()
{
	APBBossBase* Boss = GetOwnerBoss();
	ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss);
	if (!SnakeBoss)
	{
		return;
	}

	const FVector SpawnLocation = GetProjectileSpawnLocation();
	SnakeBoss->FaceHeadDirection(GetProjectileSpawnRotation(SpawnLocation).Vector());
}

void UPBBossProjectilePattern::ClearFireTimer()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}
}

void UPBBossProjectilePattern::ClearAimTimer()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(AimTimerHandle);
	}
}

void UPBBossProjectilePattern::ClearPatternTimers()
{
	ClearFireTimer();
	ClearAimTimer();
}

FVector UPBBossProjectilePattern::GetProjectileSpawnLocation() const
{
	const APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return FVector::ZeroVector;
	}

	return Boss->GetActorLocation() + Boss->GetActorTransform().TransformVectorNoScale(SpawnOffset);
}

FRotator UPBBossProjectilePattern::GetProjectileSpawnRotation(const FVector& SpawnLocation) const
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
