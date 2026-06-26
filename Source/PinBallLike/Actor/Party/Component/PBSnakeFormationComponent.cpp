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
		LeaderTrail.Add({PartyBalls[0]->GetActorLocation(), CurrentTime});
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

	LeaderTrail.Add({PartyBalls[0]->GetActorLocation(), CurrentTime});
}

void UPBSnakeFormationComponent::UpdateFollowers(float CurrentTime)
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
		if (FindTrailLocation(TargetTime, TargetLocation))
		{
			FollowerBall->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::None);
		}
	}
}

bool UPBSnakeFormationComponent::FindTrailLocation(float TargetTime, FVector& OutLocation) const
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

void UPBSnakeFormationComponent::TrimTrail(float CurrentTime)
{
	const float OldestTimeToKeep = CurrentTime - TrailKeepSeconds;
	while (LeaderTrail.Num() > 1 && LeaderTrail[0].TimeSeconds < OldestTimeToKeep)
	{
		LeaderTrail.RemoveAt(0, 1, EAllowShrinking::No);
	}
}
