// Fill out your copyright notice in the Description page of Project Settings.


#include "PBSnakeFormationComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"

UPBSnakeFormationComponent::UPBSnakeFormationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UPBSnakeFormationComponent::SetPartyBalls(const TArray<APBBallBase*>& OrderedBalls)
{
	PartyBalls.Reset();
	PartyBalls.Reserve(OrderedBalls.Num());
	for (APBBallBase* Ball : OrderedBalls)
	{
		if (IsValid(Ball))
		{
			PartyBalls.Add(Ball);
		}
	}

	LeaderTrail.Reset();
	if (PartyBalls.Num() > 0 && IsValid(PartyBalls[0].Get()))
	{
		const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		LeaderTrail.Add({PartyBalls[0]->GetActorLocation(), CurrentTime, 0.0f});
	}
}

void UPBSnakeFormationComponent::ClearFormation()
{
	PartyBalls.Reset();
	LeaderTrail.Reset();
}

void UPBSnakeFormationComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PartyBalls.Num() == 0 || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	RecordLeaderTrail(CurrentTime);
	UpdateFollowers(CurrentTime);
	TrimTrail(CurrentTime);
}

void UPBSnakeFormationComponent::RecordLeaderTrail(float CurrentTime)
{
	if (!PartyBalls.IsValidIndex(0) || !IsValid(PartyBalls[0].Get()))
	{
		return;
	}

	const FVector LeaderLocation = PartyBalls[0]->GetActorLocation();
	const float PreviousDistance = LeaderTrail.Num() > 0 ? LeaderTrail.Last().DistanceAlongTrail : 0.0f;
	const FVector PreviousLocation = LeaderTrail.Num() > 0 ? LeaderTrail.Last().Location : LeaderLocation;
	const float MovedDistance = FVector::Dist2D(PreviousLocation, LeaderLocation);

	LeaderTrail.Add({LeaderLocation, CurrentTime, PreviousDistance + MovedDistance});
}

void UPBSnakeFormationComponent::UpdateFollowers(float CurrentTime)
{
	switch (FormationMode)
	{
	case EPBSnakeFormationMode::TimeDelayWithDistanceClamp:
		UpdateFollowersByTimeDelay(CurrentTime);
		break;
	case EPBSnakeFormationMode::FixedDistance:
	default:
		UpdateFollowersByFixedDistance();
		break;
	}
}

void UPBSnakeFormationComponent::UpdateFollowersByTimeDelay(float CurrentTime)
{
	for (int32 PartyIndex = 1; PartyIndex < PartyBalls.Num(); ++PartyIndex)
	{
		APBBallBase* FollowerBall = PartyBalls[PartyIndex].Get();
		if (!IsValid(FollowerBall))
		{
			continue;
		}

		FVector TargetLocation;
		const float TargetTime = CurrentTime - FollowerDelaySeconds * PartyIndex;
		if (FindTrailLocationAtTime(TargetTime, TargetLocation))
		{
			const APBBallBase* PreviousBall = PartyBalls[PartyIndex - 1].Get();
			if (IsValid(PreviousBall))
			{
				TargetLocation = ClampTargetDistanceFromPreviousBall(PreviousBall->GetActorLocation(), TargetLocation);
			}
			FollowerBall->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::None);
		}
	}
}

void UPBSnakeFormationComponent::UpdateFollowersByFixedDistance()
{
	if (LeaderTrail.Num() == 0)
	{
		return;
	}

	const float LatestDistance = LeaderTrail.Last().DistanceAlongTrail;
	for (int32 PartyIndex = 1; PartyIndex < PartyBalls.Num(); ++PartyIndex)
	{
		APBBallBase* FollowerBall = PartyBalls[PartyIndex].Get();
		if (!IsValid(FollowerBall))
		{
			continue;
		}

		FVector TargetLocation;
		const float TargetDistance = LatestDistance - FixedFollowerDistance * PartyIndex;
		if (FindTrailLocationAtDistance(TargetDistance, TargetLocation))
		{
			FollowerBall->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::None);
		}
	}
}

bool UPBSnakeFormationComponent::FindTrailLocationAtTime(float TargetTime, FVector& OutLocation) const
{
	if (LeaderTrail.Num() == 0)
	{
		return false;
	}

	for (int32 SampleIndex = LeaderTrail.Num() - 1; SampleIndex >= 0; --SampleIndex)
	{
		if (LeaderTrail[SampleIndex].TimeSeconds <= TargetTime)
		{
			OutLocation = LeaderTrail[SampleIndex].Location;
			return true;
		}
	}

	OutLocation = LeaderTrail[0].Location;
	return true;
}

bool UPBSnakeFormationComponent::FindTrailLocationAtDistance(float TargetDistance, FVector& OutLocation) const
{
	if (LeaderTrail.Num() == 0)
	{
		return false;
	}

	if (TargetDistance <= LeaderTrail[0].DistanceAlongTrail)
	{
		OutLocation = LeaderTrail[0].Location;
		return true;
	}

	for (int32 SampleIndex = LeaderTrail.Num() - 1; SampleIndex >= 0; --SampleIndex)
	{
		const FPBSnakeTrailSample& CurrentSample = LeaderTrail[SampleIndex];
		if (CurrentSample.DistanceAlongTrail > TargetDistance)
		{
			continue;
		}

		if (!LeaderTrail.IsValidIndex(SampleIndex + 1))
		{
			OutLocation = CurrentSample.Location;
			return true;
		}

		const FPBSnakeTrailSample& NextSample = LeaderTrail[SampleIndex + 1];
		const float SegmentDistance = NextSample.DistanceAlongTrail - CurrentSample.DistanceAlongTrail;
		if (FMath::IsNearlyZero(SegmentDistance))
		{
			OutLocation = CurrentSample.Location;
			return true;
		}

		const float SegmentAlpha = FMath::Clamp(
			(TargetDistance - CurrentSample.DistanceAlongTrail) / SegmentDistance,
			0.0f,
			1.0f);
		OutLocation = FMath::Lerp(CurrentSample.Location, NextSample.Location, SegmentAlpha);
		return true;
	}

	OutLocation = LeaderTrail[0].Location;
	return true;
}

FVector UPBSnakeFormationComponent::ClampTargetDistanceFromPreviousBall(
	const FVector& PreviousBallLocation,
	const FVector& TargetLocation) const
{
	const FVector Offset = TargetLocation - PreviousBallLocation;
	const float CurrentDistance = Offset.Size2D();
	if (FMath::IsNearlyZero(CurrentDistance))
	{
		return TargetLocation;
	}

	const float SafeMinDistance = FMath::Max(MinFollowerDistance, 0.0f);
	const float SafeMaxDistance = FMath::Max(MaxFollowerDistance, SafeMinDistance);
	const float ClampedDistance = FMath::Clamp(CurrentDistance, SafeMinDistance, SafeMaxDistance);
	if (FMath::IsNearlyEqual(CurrentDistance, ClampedDistance))
	{
		return TargetLocation;
	}

	FVector Direction = Offset;
	Direction.Z = 0.0f;
	Direction.Normalize();
	return PreviousBallLocation + Direction * ClampedDistance;
}

void UPBSnakeFormationComponent::TrimTrail(float CurrentTime)
{
	const float OldestTimeToKeep = CurrentTime - TrailKeepSeconds;
	const float RequiredFollowDistance = FixedFollowerDistance * FMath::Max(PartyBalls.Num(), 1);
	const float DistanceToKeep = FMath::Max(TrailKeepDistance, RequiredFollowDistance);
	const float OldestDistanceToKeep = LeaderTrail.Num() > 0
		? LeaderTrail.Last().DistanceAlongTrail - DistanceToKeep
		: 0.0f;

	while (LeaderTrail.Num() > 1
		&& LeaderTrail[0].TimeSeconds < OldestTimeToKeep
		&& LeaderTrail[0].DistanceAlongTrail < OldestDistanceToKeep)
	{
		LeaderTrail.RemoveAt(0, 1, EAllowShrinking::No);
	}
}
