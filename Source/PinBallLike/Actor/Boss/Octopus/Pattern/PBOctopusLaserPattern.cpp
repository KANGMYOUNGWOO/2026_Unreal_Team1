#include "PBOctopusLaserPattern.h"

#include "Engine/World.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemLaserWall.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "TimerManager.h"

UPBOctopusLaserPattern::UPBOctopusLaserPattern()
{
	PatternName = TEXT("Laser");
}

bool UPBOctopusLaserPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss) && LaserWallClass != nullptr;
}

void UPBOctopusLaserPattern::ExecutePattern_Implementation(APBBossBase* Boss)
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

	DestroyLaserWall();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Boss;
	SpawnParameters.Instigator = Boss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedLaserWall = World->SpawnActor<APBGolemLaserWall>(
		LaserWallClass,
		GetLaserSpawnTransform(Boss),
		SpawnParameters);

	if (!SpawnedLaserWall)
	{
		FinishPattern();
		return;
	}

	InitialLaserScale = SpawnedLaserWall->GetActorScale3D();
	InitialLaserRotation = SpawnedLaserWall->GetActorRotation();
	ElapsedPatternTime = 0.0f;

	FVector LaserScale = InitialLaserScale;
	LaserScale.X *= FMath::Max(0.01f, InitialLengthScale);
	SpawnedLaserWall->SetActorScale3D(LaserScale);

	const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
	SpawnedLaserWall->InitializeLaserWall(
		SpawnedLaserWall->GetActorForwardVector(),
		BounceVelocity,
		SourcePatternName,
		DamageAmount);

	if (LaserDuration <= 0.0f)
	{
		CompleteLaserPattern();
		return;
	}

	World->GetTimerManager().SetTimer(
		LaserTimerHandle,
		this,
		&UPBOctopusLaserPattern::CompleteLaserPattern,
		LaserDuration,
		false);

	World->GetTimerManager().SetTimer(
		UpdateTimerHandle,
		this,
		&UPBOctopusLaserPattern::UpdateLaserSweep,
		FMath::Max(0.01f, UpdateInterval),
		true);
}

void UPBOctopusLaserPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	static_cast<void>(Boss);

	ClearLaserTimers();
	DestroyLaserWall();
}

void UPBOctopusLaserPattern::CompleteLaserPattern()
{
	ClearLaserTimers();
	DestroyLaserWall();
	FinishPattern();
}

void UPBOctopusLaserPattern::UpdateLaserSweep()
{
	if (!IsValid(SpawnedLaserWall))
	{
		return;
	}

	const float SafeUpdateInterval = FMath::Max(0.01f, UpdateInterval);
	ElapsedPatternTime += SafeUpdateInterval;

	const float GrowthAlpha = LaserGrowthDuration <= 0.0f
		? 1.0f
		: FMath::Clamp(ElapsedPatternTime / LaserGrowthDuration, 0.0f, 1.0f);
	const float LengthScale = FMath::Lerp(
		FMath::Max(0.01f, InitialLengthScale),
		FMath::Max(0.01f, MaximumLengthScale),
		GrowthAlpha);

	FVector LaserScale = InitialLaserScale;
	LaserScale.X *= LengthScale;
	SpawnedLaserWall->SetActorScale3D(LaserScale);

	const FRotator SweepRotation(0.0f, RotationSpeed * ElapsedPatternTime, 0.0f);
	SpawnedLaserWall->SetActorRotation(InitialLaserRotation + SweepRotation);

	const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
	SpawnedLaserWall->InitializeLaserWall(
		SpawnedLaserWall->GetActorForwardVector(),
		BounceVelocity,
		SourcePatternName,
		DamageAmount);
}

void UPBOctopusLaserPattern::ClearLaserTimers()
{
	if (const APBBossBase* Boss = GetOwnerBoss())
	{
		if (UWorld* World = Boss->GetWorld())
		{
			World->GetTimerManager().ClearTimer(LaserTimerHandle);
			World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		}
	}
}

void UPBOctopusLaserPattern::DestroyLaserWall()
{
	if (IsValid(SpawnedLaserWall))
	{
		SpawnedLaserWall->Destroy();
	}

	SpawnedLaserWall = nullptr;
}

FTransform UPBOctopusLaserPattern::GetLaserSpawnTransform(APBBossBase* Boss) const
{
	const FTransform BossTransform = Boss ? Boss->GetActorTransform() : FTransform::Identity;
	const FVector SpawnLocation = BossTransform.TransformPosition(
		LaserOffset + FVector(LaserSpawnDistance, 0.0f, 0.0f));
	const FRotator SpawnRotation = BossTransform.GetRotation().Rotator() + LaserRotationOffset;

	return FTransform(SpawnRotation, SpawnLocation);
}
