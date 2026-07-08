#include "PBBossMoveArea.h"

#include "Components/BoxComponent.h"

APBBossMoveArea::APBBossMoveArea()
{
	PrimaryActorTick.bCanEverTick = false;

	AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	SetRootComponent(AreaBox);

	AreaBox->InitBoxExtent(FVector(700.0f, 700.0f, 100.0f));
	AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaBox->SetGenerateOverlapEvents(false);
	AreaBox->SetHiddenInGame(true);
	AreaBox->SetVisibility(true);
}

FVector APBBossMoveArea::GetAreaCenter() const
{
	return GetActorLocation();
}

FVector APBBossMoveArea::GetAreaExtent() const
{
	return AreaBox ? AreaBox->GetScaledBoxExtent() : FVector::ZeroVector;
}

FVector APBBossMoveArea::ClampLocation(const FVector& SourceLocation) const
{
	const FVector AreaCenter = GetAreaCenter();
	const FVector AreaExtent = GetAreaExtent().GetAbs();

	FVector ClampedLocation = SourceLocation;
	ClampedLocation.X = FMath::Clamp(ClampedLocation.X, AreaCenter.X - AreaExtent.X, AreaCenter.X + AreaExtent.X);
	ClampedLocation.Y = FMath::Clamp(ClampedLocation.Y, AreaCenter.Y - AreaExtent.Y, AreaCenter.Y + AreaExtent.Y);
	return ClampedLocation;
}
