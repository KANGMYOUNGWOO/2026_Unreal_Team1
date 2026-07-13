#include "PBCombatCameraTrackingComponent.h"

#include "EngineUtils.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"

DEFINE_LOG_CATEGORY_STATIC(LogPBCombatCamera, Log, All);

namespace
{
constexpr float LaunchReadyExactTolerance = 0.1f;
constexpr float LaunchReadySnapTolerance = 1.0f;
}

UPBCombatCameraTrackingComponent::UPBCombatCameraTrackingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UPBCombatCameraTrackingComponent::BeginPlay()
{
	Super::BeginPlay();

	NormalizeConfiguration();

	if (const AActor* Owner = GetOwner())
	{
		TrackingOrigin = Owner->GetActorLocation();
		TrackingOriginCoordinate = FVector::DotProduct(TrackingOrigin, NormalizedTrackingAxis);
	}

	if (bTrackingEnabled)
	{
		RefreshTrackingSources();
	}
	SetComponentTickEnabled(bTrackingEnabled);

	UE_LOG(LogPBCombatCamera, Log,
		TEXT("Initialized. Owner=%s Enabled=%s Axis=%s Bounds=[%.2f, %.2f] DeadZone=%.2f MaxSpeed=%.2f"),
		*GetNameSafe(GetOwner()),
		bTrackingEnabled ? TEXT("true") : TEXT("false"),
		*NormalizedTrackingAxis.ToCompactString(),
		MinimumTrackingCoordinate,
		MaximumTrackingCoordinate,
		TrackingDeadZone,
		MaximumFollowSpeed);
}

void UPBCombatCameraTrackingComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!bTrackingEnabled || DeltaTime <= 0.0f || !IsValid(Owner))
	{
		return;
	}

	SourceRefreshElapsedTime += DeltaTime;
	if (SourceRefreshElapsedTime >= SourceRefreshInterval)
	{
		RefreshTrackingSources();
	}

	float DesiredCoordinate = 0.0f;
	EPBCombatCameraTrackingMode NewTrackingMode = EPBCombatCameraTrackingMode::None;
	ResolveDesiredCoordinate(DesiredCoordinate, NewTrackingMode);

	SetCurrentTrackingMode(NewTrackingMode);
	DesiredCoordinate = FMath::Clamp(
		DesiredCoordinate,
		MinimumTrackingCoordinate,
		MaximumTrackingCoordinate);

	const float CurrentCoordinate = FVector::DotProduct(
		Owner->GetActorLocation(),
		NormalizedTrackingAxis);
	const float DistanceToTarget = DesiredCoordinate - CurrentCoordinate;
	const bool bIsLaunchReady =
		NewTrackingMode == EPBCombatCameraTrackingMode::LaunchReady;
	if (!bIsLaunchReady && FMath::Abs(DistanceToTarget) <= TrackingDeadZone)
	{
		return;
	}
	if (bIsLaunchReady && FMath::IsNearlyEqual(
		CurrentCoordinate,
		DesiredCoordinate,
		LaunchReadyExactTolerance))
	{
		ApplyTrackingCoordinate(DesiredCoordinate);
		return;
	}

	const float InterpolationTarget = bIsLaunchReady
		? DesiredCoordinate
		: DesiredCoordinate - FMath::Sign(DistanceToTarget) * TrackingDeadZone;
	const float InterpolatedCoordinate = FMath::FInterpTo(
		CurrentCoordinate,
		InterpolationTarget,
		DeltaTime,
		FollowInterpSpeed);
	const float MaximumStep = MaximumFollowSpeed * DeltaTime;
	const float LimitedStep = FMath::Clamp(
		InterpolatedCoordinate - CurrentCoordinate,
		-MaximumStep,
		MaximumStep);
	float NewCoordinate = FMath::Clamp(
		CurrentCoordinate + LimitedStep,
		MinimumTrackingCoordinate,
		MaximumTrackingCoordinate);
	if (bIsLaunchReady && FMath::IsNearlyEqual(
		NewCoordinate,
		DesiredCoordinate,
		LaunchReadySnapTolerance))
	{
		NewCoordinate = DesiredCoordinate;
	}

	ApplyTrackingCoordinate(NewCoordinate);
}

void UPBCombatCameraTrackingComponent::SetTrackingEnabled(const bool bEnabled)
{
	bTrackingEnabled = bEnabled;
	SetComponentTickEnabled(bTrackingEnabled);

	if (bTrackingEnabled)
	{
		RefreshTrackingSources();
	}
}

void UPBCombatCameraTrackingComponent::SetTrackingBounds(
	const float NewMinimumCoordinate,
	const float NewMaximumCoordinate)
{
	MinimumTrackingCoordinate = FMath::Min(NewMinimumCoordinate, NewMaximumCoordinate);
	MaximumTrackingCoordinate = FMath::Max(NewMinimumCoordinate, NewMaximumCoordinate);

	if (const AActor* Owner = GetOwner())
	{
		const float CurrentCoordinate = FVector::DotProduct(
			Owner->GetActorLocation(),
			NormalizedTrackingAxis);
		ApplyTrackingCoordinate(FMath::Clamp(
			CurrentCoordinate,
			MinimumTrackingCoordinate,
			MaximumTrackingCoordinate));
	}
}

void UPBCombatCameraTrackingComponent::RefreshTrackingSources()
{
	SourceRefreshElapsedTime = 0.0f;

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	if (!CachedPartyController.IsValid())
	{
		for (TActorIterator<APBCombatPartyController> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				CachedPartyController = *It;
				break;
			}
		}
	}

	CachedFallbackBall.Reset();
	if (!CachedPartyController.IsValid())
	{
		APBBallBase* FirstTrackableBall = nullptr;
		for (TActorIterator<APBBallBase> It(World); It; ++It)
		{
			APBBallBase* Candidate = *It;
			if (!IsTrackableBall(Candidate))
			{
				continue;
			}

			if (Candidate->GetCombatRole() == EPBBallPartyRole::Leader)
			{
				CachedFallbackBall = Candidate;
				break;
			}

			if (!FirstTrackableBall)
			{
				FirstTrackableBall = Candidate;
			}
		}

		if (!CachedFallbackBall.IsValid())
		{
			CachedFallbackBall = FirstTrackableBall;
		}
	}

	if (!IsTrackableBoss(CachedBoss.Get()))
	{
		CachedBoss.Reset();
		for (TActorIterator<APBBossBase> It(World); It; ++It)
		{
			if (IsTrackableBoss(*It))
			{
				CachedBoss = *It;
				break;
			}
		}
	}
}

void UPBCombatCameraTrackingComponent::ResolveDesiredCoordinate(
	float& OutCoordinate,
	EPBCombatCameraTrackingMode& OutMode) const
{
	const APBBallBase* Ball = GetTrackableBall();
	if (!Ball)
	{
		OutCoordinate = MinimumTrackingCoordinate;
		OutMode = EPBCombatCameraTrackingMode::LaunchReady;
		return;
	}

	const APBBossBase* Boss = GetTrackableBoss();
	if (Boss)
	{
		const float BallCoordinate = FVector::DotProduct(
			Ball->GetActorLocation(),
			NormalizedTrackingAxis);
		const float BossCoordinate = FVector::DotProduct(
			Boss->GetActorLocation(),
			NormalizedTrackingAxis);
		OutCoordinate = FMath::Lerp(BallCoordinate, BossCoordinate, BallBossBlendAlpha);
		OutMode = EPBCombatCameraTrackingMode::BallAndBoss;
		return;
	}

	OutCoordinate = FVector::DotProduct(Ball->GetActorLocation(), NormalizedTrackingAxis);
	OutMode = EPBCombatCameraTrackingMode::Ball;
}

APBBallBase* UPBCombatCameraTrackingComponent::GetTrackableBall() const
{
	APBBallBase* Ball = CachedPartyController.IsValid()
		? CachedPartyController->GetLeaderBall()
		: CachedFallbackBall.Get();
	return IsTrackableBall(Ball) ? Ball : nullptr;
}

APBBossBase* UPBCombatCameraTrackingComponent::GetTrackableBoss() const
{
	APBBossBase* Boss = CachedBoss.Get();
	return IsTrackableBoss(Boss) ? Boss : nullptr;
}

bool UPBCombatCameraTrackingComponent::IsTrackableBall(const APBBallBase* Ball) const
{
	return IsValid(Ball)
		&& !Ball->IsActorBeingDestroyed()
		&& !Ball->IsHidden();
}

bool UPBCombatCameraTrackingComponent::IsTrackableBoss(const APBBossBase* Boss) const
{
	return IsValid(Boss)
		&& !Boss->IsActorBeingDestroyed()
		&& !Boss->IsHidden()
		&& !Boss->IsDeadState();
}

void UPBCombatCameraTrackingComponent::ApplyTrackingCoordinate(const float NewCoordinate) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector NewLocation = TrackingOrigin
		+ NormalizedTrackingAxis * (NewCoordinate - TrackingOriginCoordinate);
	Owner->SetActorLocation(NewLocation, false, nullptr, ETeleportType::None);
}

void UPBCombatCameraTrackingComponent::NormalizeConfiguration()
{
	NormalizedTrackingAxis = TrackingAxis;
	NormalizedTrackingAxis.Z = 0.0f;
	if (!NormalizedTrackingAxis.Normalize())
	{
		NormalizedTrackingAxis = FVector::ForwardVector;
	}

	if (MinimumTrackingCoordinate > MaximumTrackingCoordinate)
	{
		Swap(MinimumTrackingCoordinate, MaximumTrackingCoordinate);
	}

	BallBossBlendAlpha = FMath::Clamp(BallBossBlendAlpha, 0.0f, 1.0f);
	TrackingDeadZone = FMath::Max(TrackingDeadZone, 0.0f);
	FollowInterpSpeed = FMath::Max(FollowInterpSpeed, 0.0f);
	MaximumFollowSpeed = FMath::Max(MaximumFollowSpeed, 0.0f);
	SourceRefreshInterval = FMath::Max(SourceRefreshInterval, 0.05f);
}

void UPBCombatCameraTrackingComponent::SetCurrentTrackingMode(
	const EPBCombatCameraTrackingMode NewMode)
{
	if (CurrentTrackingMode == NewMode)
	{
		return;
	}

	CurrentTrackingMode = NewMode;
	UE_LOG(LogPBCombatCamera, Log,
		TEXT("Tracking mode changed. Owner=%s Mode=%s"),
		*GetNameSafe(GetOwner()),
		*UEnum::GetValueAsString(CurrentTrackingMode));
}
