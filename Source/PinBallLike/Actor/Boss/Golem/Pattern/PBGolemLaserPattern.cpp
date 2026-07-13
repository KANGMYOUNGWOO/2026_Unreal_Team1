#include "PBGolemLaserPattern.h"

#include "Engine/World.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemLaserWall.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "TimerManager.h"

bool UPBGolemLaserPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	const bool IsCanExecute = Super::CanExecute_Implementation(Boss) && LaserWallClass != nullptr;
	return IsCanExecute;
}

void UPBGolemLaserPattern::StartPattern_Implementation(APBBossBase* Boss)
{
	SetOwnerBoss(Boss);

	if (!Boss || !LaserWallClass)
	{
		FinishPattern();
		return;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		FinishPattern();
		return;
	}

	const FVector LaserDirection = GetLaserDirection(Boss);

	DestroyLaserWall();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Boss;
	SpawnParameters.Instigator = Boss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedLaserWall = World->SpawnActor<APBGolemLaserWall>(
		LaserWallClass,
		GetLaserWallSpawnTransform(Boss),
		SpawnParameters);

	if (!SpawnedLaserWall)
	{
		FinishPattern();
		return;
	}

	SpawnedLaserWall->InitializeLaserWall(LaserDirection, BounceVelocity);

	if (LaserWallDuration <= 0.0f)
	{
		HandleLaserWallFinished();
		return;
	}

	World->GetTimerManager().SetTimer(
		LaserWallTimerHandle,
		this,
		&UPBGolemLaserPattern::HandleLaserWallFinished,
		LaserWallDuration,
		false);
}

void UPBGolemLaserPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	static_cast<void>(Boss);

	ClearLaserWallTimer();
	DestroyLaserWall();
}

void UPBGolemLaserPattern::HandleLaserWallFinished()
{
	ClearLaserWallTimer();
	DestroyLaserWall();
	FinishPattern();
}

void UPBGolemLaserPattern::ClearLaserWallTimer()
{
	if (const APBBossBase* Boss = GetOwnerBoss())
	{
		if (UWorld* World = Boss->GetWorld())
		{
			World->GetTimerManager().ClearTimer(LaserWallTimerHandle);
		}
	}
}

void UPBGolemLaserPattern::DestroyLaserWall()
{
	if (IsValid(SpawnedLaserWall))
	{
		SpawnedLaserWall->Destroy();
	}

	SpawnedLaserWall = nullptr;
}

FVector UPBGolemLaserPattern::GetLaserDirection(APBBossBase* Boss) const
{
	if (!Boss)
	{
		return FVector::ForwardVector;
	}

	const AActor* PinballActor = FindPinballActor(Boss);
	if (!IsValid(PinballActor))
	{
		return Boss->GetActorForwardVector().GetSafeNormal2D();
	}

	FVector LaserDirection = PinballActor->GetActorLocation() - Boss->GetActorLocation();
	LaserDirection.Z = 0.0f;
	LaserDirection = LaserDirection.GetSafeNormal();

	if (LaserDirection.IsNearlyZero())
	{
		return Boss->GetActorForwardVector().GetSafeNormal2D();
	}

	return LaserDirection;
}

FTransform UPBGolemLaserPattern::GetLaserWallSpawnTransform(APBBossBase* Boss) const
{
	const FTransform BossTransform = Boss ? Boss->GetActorTransform() : FTransform::Identity;
	const FVector LaserDirection = GetLaserDirection(Boss);
	const FVector SpawnLocation = BossTransform.TransformPosition(LaserWallOffset);
	const FRotator SpawnRotation = (LaserDirection.IsNearlyZero()
		? BossTransform.GetRotation().Rotator()
		: LaserDirection.Rotation()) + LaserWallRotationOffset;
	return FTransform(SpawnRotation, SpawnLocation);
}
