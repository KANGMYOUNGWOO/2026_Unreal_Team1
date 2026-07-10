// Fill out your copyright notice in the Description page of Project Settings.


#include "PBLeaderPromotionComponent.h"

#include "Engine/EngineTypes.h"
#include "PBSnakeFormationComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

UPBLeaderPromotionComponent::UPBLeaderPromotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPBLeaderPromotionComponent::TickComponent(
	const float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bPromotionInProgress)
	{
		return;
	}

	if (!IsValid(PromotingLeaderBall.Get()))
	{
		FinishPromotion();
		return;
	}

	PromotionElapsedTime += FMath::Max(DeltaTime, 0.0f);
	const float SafeDuration = FMath::Max(PromotionDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(PromotionElapsedTime / SafeDuration, 0.0f, 1.0f);
	const float SmoothAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);

	PromotingLeaderBall->SetActorLocation(
		FMath::Lerp(PromotionStartLocation, PromotionTargetLocation, SmoothAlpha),
		false,
		nullptr,
		ETeleportType::None);

	if (Alpha >= 1.0f)
	{
		FinishPromotion();
	}
}

void UPBLeaderPromotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPromotion();
	Super::EndPlay(EndPlayReason);
}

void UPBLeaderPromotionComponent::InitializeDependencies(
	FPBLeaderPromotionDependencies InDependencies,
	UPBSnakeFormationComponent* InSnakeFormationComponent)
{
	Dependencies = MoveTemp(InDependencies);
	SnakeFormationComponent = InSnakeFormationComponent;
}

void UPBLeaderPromotionComponent::StartLeaderPromotion(
	APBBallBase* NewLeaderBall,
	const FVector TargetLocation,
	FVector InPromotionInheritedVelocity)
{
	if (!IsValid(NewLeaderBall)
		|| !Dependencies.RebuildPartyRoles
		|| !Dependencies.ClearPartyRoles
		|| !Dependencies.ApplyPartyRoles
		|| !Dependencies.GetValidPartyBalls)
	{
		return;
	}

	InPromotionInheritedVelocity.Z = 0.0f;
	Dependencies.RebuildPartyRoles();
	Dependencies.ClearPartyRoles();
	if (SnakeFormationComponent)
	{
		SnakeFormationComponent->SetComponentTickEnabled(false);
	}

	PromotingLeaderBall = NewLeaderBall;
	PromotionStartLocation = NewLeaderBall->GetActorLocation();
	PromotionTargetLocation = TargetLocation;
	InheritedVelocity = InPromotionInheritedVelocity;
	PromotionElapsedTime = 0.0f;
	bPromotionInProgress = true;
	SetComponentTickEnabled(true);

	UE_LOG(LogTemp, Warning, TEXT("[LeaderPromotion] Promotion started. Owner=%s NewLeader=%s From=%s To=%s InheritedVelocity=%s Duration=%.2f"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(NewLeaderBall),
		*PromotionStartLocation.ToString(),
		*PromotionTargetLocation.ToString(),
		*InheritedVelocity.ToString(),
		PromotionDuration);
}

void UPBLeaderPromotionComponent::StopPromotion()
{
	bPromotionInProgress = false;
	PromotionElapsedTime = 0.0f;
	PromotingLeaderBall = nullptr;
	InheritedVelocity = FVector::ZeroVector;
	SetComponentTickEnabled(false);
}

void UPBLeaderPromotionComponent::FinishPromotion()
{
	APBBallBase* FinishedLeaderBall = PromotingLeaderBall.Get();
	const FVector FinishedInheritedVelocity = InheritedVelocity;

	StopPromotion();

	if (IsValid(FinishedLeaderBall) && !FinishedInheritedVelocity.IsNearlyZero())
	{
		if (IMovable* NewLeaderMovable = PBInterfaceUtils::FindInterface<IMovable>(FinishedLeaderBall))
		{
			NewLeaderMovable->AddVelocity(FinishedInheritedVelocity);
		}
	}

	if (Dependencies.RebuildPartyRoles && Dependencies.ApplyPartyRoles && Dependencies.GetValidPartyBalls)
	{
		Dependencies.RebuildPartyRoles();
		if (SnakeFormationComponent)
		{
			SnakeFormationComponent->SetPartyBalls(Dependencies.GetValidPartyBalls());
			SnakeFormationComponent->SetComponentTickEnabled(true);
		}
		Dependencies.ApplyPartyRoles();
	}

	UE_LOG(LogTemp, Warning, TEXT("[LeaderPromotion] Promotion finished. Owner=%s Leader=%s InheritedVelocity=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(FinishedLeaderBall),
		*FinishedInheritedVelocity.ToString());

	OnPromotionFinished.Broadcast();
}
