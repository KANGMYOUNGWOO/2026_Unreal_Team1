#include "PBBossChargeTelegraph.h"

void APBBossChargeTelegraph::InitChargeTelegraph(
	float InDurationSeconds,
	const FVector& StartLocation,
	const FVector& Direction,
	float Length)
{
	UpdateChargeTelegraphTransform(StartLocation, Direction, Length);
	InitTelegraph(InDurationSeconds, GetActorScale3D());
}

void APBBossChargeTelegraph::UpdateChargeTelegraphTransform(
	const FVector& StartLocation,
	const FVector& Direction,
	float Length)
{
	FVector SafeDirection = Direction;
	SafeDirection.Z = 0.0f;
	SafeDirection = SafeDirection.GetSafeNormal();

	if (SafeDirection.IsNearlyZero())
	{
		SafeDirection = FVector::ForwardVector;
	}

	const float SafeLength = FMath::Max(0.0f, Length);

	SetActorLocation(StartLocation + SafeDirection * SafeLength * 0.5f);
	SetActorRotation(SafeDirection.Rotation());
}
