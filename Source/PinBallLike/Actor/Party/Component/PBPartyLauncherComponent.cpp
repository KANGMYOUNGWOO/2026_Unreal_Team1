// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPartyLauncherComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"

UPBPartyLauncherComponent::UPBPartyLauncherComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPBPartyLauncherComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheLauncherBaseLocation();
	SetLauncherActive(bLauncherActive);
}

void UPBPartyLauncherComponent::TickComponent(
	const float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bLauncherActive)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	LauncherElapsedTime += FMath::Max(DeltaTime, 0.0f);
	const float MoveOffset = FMath::Sin(LauncherElapsedTime * LauncherMoveSpeed * 2.0f * PI) * LauncherMoveHalfRange;
	Owner->SetActorLocation(LauncherBaseLocation + ResolveHorizontalMoveAxis() * MoveOffset);
}

void UPBPartyLauncherComponent::InitializeDependencies(FPBPartyLauncherDependencies InDependencies)
{
	Dependencies = MoveTemp(InDependencies);
}

void UPBPartyLauncherComponent::SetLauncherVisualComponent(UStaticMeshComponent* InLauncherVisualComponent)
{
	LauncherVisualComponent = InLauncherVisualComponent;
	SetLauncherActive(bLauncherActive);
}

void UPBPartyLauncherComponent::SetLauncherActive(const bool bNewLauncherActive)
{
	bLauncherActive = bNewLauncherActive;
	SetComponentTickEnabled(bLauncherActive);

	SetSnakeFormationActive(!bLauncherActive);
	SetLauncherVisualActive(bLauncherActive);

	if (bLauncherActive)
	{
		CacheLauncherBaseLocation();
		HidePartyBallsForLaunchReady();
	}
}

bool UPBPartyLauncherComponent::LaunchPartyFromReadyPosition()
{
	AActor* Owner = GetOwner();
	APBBallBase* LeaderBall = GetLeaderBall();
	const TArray<TObjectPtr<APBBallBase>>& PartyBalls = GetPartyBalls();

	UE_LOG(LogTemp, Warning, TEXT("LaunchPartyFromReadyPosition called. Party=%s bLauncherActive=%s Leader=%s PartyBallCount=%d"),
		*GetNameSafe(Owner),
		IsLauncherActive() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(LeaderBall),
		PartyBalls.Num());

	if (!IsValid(LeaderBall))
	{
		UE_LOG(LogTemp, Warning, TEXT("LaunchPartyFromReadyPosition failed: LeaderBall is invalid."));
		return false;
	}

	ResetPartyDeathState();
	SetLauncherActive(false);
	PlacePartyBallsAtLauncher();
	RefreshPartyOrder();
	BindPartyDeathEvents();

	UE_LOG(LogTemp, Warning, TEXT("LaunchPartyFromReadyPosition succeeded. Party balls placed at launcher."));
	return true;
}

void UPBPartyLauncherComponent::HidePartyBallsForLaunchReady() const
{
	for (const TObjectPtr<APBBallBase>& Ball : GetPartyBalls())
	{
		if (!IsValid(Ball.Get()))
		{
			continue;
		}

		Ball->SetCombatRole(EPBBallPartyRole::None);
		Ball->SetActorHiddenInGame(true);
	}
}

void UPBPartyLauncherComponent::PlacePartyBallsAtLauncher() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const TArray<TObjectPtr<APBBallBase>>& PartyBalls = GetPartyBalls();
	const int32 BallCount = PartyBalls.Num();
	UE_LOG(LogTemp, Warning, TEXT("[PartyLauncher] Place party balls. Party=%s BallCount=%d Location=%s"),
		*GetNameSafe(Owner),
		BallCount,
		*Owner->GetActorLocation().ToString());

	if (BallCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyLauncher] Place party balls skipped because PartyBalls is empty."));
		return;
	}

	const FVector SpacingAxis = ResolveHorizontalSpacingAxis();
	const float CenterIndex = static_cast<float>(BallCount - 1) * 0.5f;
	for (int32 BallIndex = 0; BallIndex < BallCount; ++BallIndex)
	{
		APBBallBase* Ball = PartyBalls[BallIndex].Get();
		if (!IsValid(Ball))
		{
			UE_LOG(LogTemp, Warning, TEXT("[PartyLauncher] Place party balls skipped invalid ball at index %d."), BallIndex);
			continue;
		}

		const float Offset = (static_cast<float>(BallIndex) - CenterIndex) * ReadyBallSpacing;
		Ball->SetActorLocation(Owner->GetActorLocation() + SpacingAxis * Offset, false, nullptr, ETeleportType::TeleportPhysics);
		Ball->SetActorHiddenInGame(false);
		UE_LOG(LogTemp, Warning, TEXT("[PartyLauncher] Placed party ball. Index=%d Ball=%s Location=%s"),
			BallIndex,
			*GetNameSafe(Ball),
			*Ball->GetActorLocation().ToString());
	}
}

const TArray<TObjectPtr<APBBallBase>>& UPBPartyLauncherComponent::GetPartyBalls() const
{
	return Dependencies.GetPartyBalls ? Dependencies.GetPartyBalls() : EmptyPartyBalls;
}

APBBallBase* UPBPartyLauncherComponent::GetLeaderBall() const
{
	return Dependencies.GetLeaderBall ? Dependencies.GetLeaderBall() : nullptr;
}

void UPBPartyLauncherComponent::ResetPartyDeathState() const
{
	if (Dependencies.ResetPartyDeathState)
	{
		Dependencies.ResetPartyDeathState();
	}
}

void UPBPartyLauncherComponent::BindPartyDeathEvents() const
{
	if (Dependencies.BindPartyDeathEvents)
	{
		Dependencies.BindPartyDeathEvents();
	}
}

void UPBPartyLauncherComponent::RefreshPartyOrder() const
{
	if (Dependencies.RefreshPartyOrder)
	{
		Dependencies.RefreshPartyOrder();
	}
}

void UPBPartyLauncherComponent::SetSnakeFormationActive(const bool bActive) const
{
	if (Dependencies.SetSnakeFormationActive)
	{
		Dependencies.SetSnakeFormationActive(bActive);
	}
}

void UPBPartyLauncherComponent::CacheLauncherBaseLocation()
{
	if (const AActor* Owner = GetOwner())
	{
		LauncherBaseLocation = Owner->GetActorLocation();
	}
}

void UPBPartyLauncherComponent::SetLauncherVisualActive(const bool bVisible) const
{
	if (!LauncherVisualComponent)
	{
		return;
	}

	LauncherVisualComponent->SetHiddenInGame(!bVisible);
	LauncherVisualComponent->SetVisibility(bVisible);
}

FVector UPBPartyLauncherComponent::ResolveHorizontalMoveAxis() const
{
	const AActor* Owner = GetOwner();
	FVector MoveAxis = Owner
		? Owner->GetActorTransform().TransformVectorNoScale(LauncherMoveLocalAxis)
		: FVector::RightVector;
	MoveAxis.Z = 0.0f;
	if (!MoveAxis.Normalize())
	{
		MoveAxis = FVector::RightVector;
	}
	return MoveAxis;
}

FVector UPBPartyLauncherComponent::ResolveHorizontalSpacingAxis() const
{
	const AActor* Owner = GetOwner();
	FVector SpacingAxis = Owner ? Owner->GetActorRightVector() : FVector::RightVector;
	SpacingAxis.Z = 0.0f;
	if (!SpacingAxis.Normalize())
	{
		SpacingAxis = FVector::RightVector;
	}
	return SpacingAxis;
}

