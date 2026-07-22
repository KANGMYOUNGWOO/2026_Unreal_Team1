#include "PBOctopusBindingPattern.h"

#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossMoveArea.h"
#include "PinBallLike/Actor/Boss/Octopus/Pattern/PBOctopusBindingZone.h"

UPBOctopusBindingPattern::UPBOctopusBindingPattern()
{
	PatternName = TEXT("Binding");
	BindingZoneClass = APBOctopusBindingZone::StaticClass();
}

bool UPBOctopusBindingPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss)
		&& BindingZoneClass
		&& FindNearestMoveArea(Boss);
}

void UPBOctopusBindingPattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	APBBossMoveArea* MoveArea = FindNearestMoveArea(Boss);
	UWorld* World = Boss ? Boss->GetWorld() : nullptr;
	if (!World || !MoveArea || !BindingZoneClass)
	{
		FinishPattern();
		return;
	}

	const FTransform ZoneTransform(FRotator::ZeroRotator, SelectZoneLocation(MoveArea));
	APBOctopusBindingZone* BindingZone = World->SpawnActorDeferred<APBOctopusBindingZone>(
		BindingZoneClass,
		ZoneTransform,
		Boss,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!BindingZone)
	{
		FinishPattern();
		return;
	}

	BindingZone->OnBallBound.AddUniqueDynamic(this, &UPBOctopusBindingPattern::HandleBallBound);

	BindingZone->InitializeZone(
		ZoneRadius,
		ZoneDuration,
		BindDuration,
		BindDamage,
		ZoneEffectScale);
	BindingZone->FinishSpawning(ZoneTransform);

	FinishPattern();
}

void UPBOctopusBindingPattern::HandleBallBound(APBBossBase* Boss)
{
	PlayPatternSFX(Boss);
}

APBBossMoveArea* UPBOctopusBindingPattern::FindNearestMoveArea(const APBBossBase* Boss) const
{
	if (!Boss)
	{
		return nullptr;
	}

	TArray<AActor*> MoveAreaActors;
	UGameplayStatics::GetAllActorsOfClass(Boss, APBBossMoveArea::StaticClass(), MoveAreaActors);

	APBBossMoveArea* NearestMoveArea = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();
	for (AActor* MoveAreaActor : MoveAreaActors)
	{
		APBBossMoveArea* MoveArea = Cast<APBBossMoveArea>(MoveAreaActor);
		if (!MoveArea)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(
			Boss->GetActorLocation(),
			MoveArea->GetAreaCenter());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestMoveArea = MoveArea;
			NearestDistanceSquared = DistanceSquared;
		}
	}

	return NearestMoveArea;
}

FVector UPBOctopusBindingPattern::SelectZoneLocation(const APBBossMoveArea* MoveArea) const
{
	const FVector AreaCenter = MoveArea->GetAreaCenter();
	const FVector AreaExtent = MoveArea->GetAreaExtent().GetAbs();
	const float AvailableExtentX = FMath::Max(AreaExtent.X - ZoneRadius, 0.0f);
	const float AvailableExtentY = FMath::Max(AreaExtent.Y - ZoneRadius, 0.0f);

	return AreaCenter + FVector(
		FMath::FRandRange(-AvailableExtentX, AvailableExtentX),
		FMath::FRandRange(-AvailableExtentY, AvailableExtentY),
		ZoneHeightOffset);
}
