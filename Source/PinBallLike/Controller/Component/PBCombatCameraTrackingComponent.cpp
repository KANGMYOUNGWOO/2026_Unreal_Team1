#include "PBCombatCameraTrackingComponent.h"

#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
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

	if (AActor* Owner = GetOwner())
	{
		TrackingOrigin = Owner->GetActorLocation();
		TrackingOriginCoordinate = FVector::DotProduct(TrackingOrigin, NormalizedTrackingAxis);
		CachedCameraComponent = Owner->FindComponentByClass<UCameraComponent>();
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

void UPBCombatCameraTrackingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndSkillFocus();
	Super::EndPlay(EndPlayReason);
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

	UpdateSkillFocus();

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
	const bool bIsSkillFocus = bSkillFocusActive && SkillFocusBall.IsValid();
	const float EffectiveDeadZone = bIsSkillFocus ? 0.0f : TrackingDeadZone;
	if (!bIsLaunchReady && FMath::Abs(DistanceToTarget) <= EffectiveDeadZone)
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
		: DesiredCoordinate - FMath::Sign(DistanceToTarget) * EffectiveDeadZone;
	const float EffectiveFollowInterpSpeed = bIsSkillFocus
		? FMath::Max(SkillFocusSettings.FollowInterpSpeed, 0.0f)
		: FollowInterpSpeed;
	const float InterpolatedCoordinate = FMath::FInterpTo(
		CurrentCoordinate,
		InterpolationTarget,
		DeltaTime,
		EffectiveFollowInterpSpeed);
	const float EffectiveMaximumFollowSpeed = bIsSkillFocus
		? FMath::Max(SkillFocusSettings.MaximumFollowSpeed, 0.0f)
		: MaximumFollowSpeed;
	const float MaximumStep = EffectiveMaximumFollowSpeed * DeltaTime;
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

bool UPBCombatCameraTrackingComponent::PlaySkillFocus(APBBallBase* TargetBall)
{
	if (!bTrackingEnabled || !IsTrackableBall(TargetBall))
	{
		return false;
	}

	if (!CachedCameraComponent.IsValid())
	{
		if (AActor* Owner = GetOwner())
		{
			CachedCameraComponent = Owner->FindComponentByClass<UCameraComponent>();
		}
	}
	if (!CachedCameraComponent.IsValid())
	{
		return false;
	}

	SkillFocusBall = TargetBall;
	if (const UWorld* World = GetWorld())
	{
		LastSkillFocusRealTime = World->GetRealTimeSeconds();
	}
	if (bSkillFocusActive)
	{
		SkillFocusElapsedTime = FMath::Max(
			SkillFocusSettings.BlendInDuration,
			0.0f);
	}
	else
	{
		SkillFocusBaseFieldOfView = CachedCameraComponent->FieldOfView;
		SkillFocusElapsedTime = 0.0f;
		bSkillFocusActive = true;
	}

	UE_LOG(LogPBCombatCamera, Log,
		TEXT("Skill focus started. Ball=%s BaseFOV=%.2f"),
		*GetNameSafe(TargetBall),
		SkillFocusBaseFieldOfView);
	return true;
}

void UPBCombatCameraTrackingComponent::SetTrackingEnabled(const bool bEnabled)
{
	if (!bEnabled)
	{
		EndSkillFocus();
	}

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
	if (bSkillFocusActive && IsTrackableBall(SkillFocusBall.Get()))
	{
		OutCoordinate = FVector::DotProduct(
			SkillFocusBall->GetActorLocation(),
			NormalizedTrackingAxis);
		OutMode = EPBCombatCameraTrackingMode::Ball;
		return;
	}

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

void UPBCombatCameraTrackingComponent::UpdateSkillFocus()
{
	if (!bSkillFocusActive)
	{
		return;
	}

	if (!IsTrackableBall(SkillFocusBall.Get()) || !CachedCameraComponent.IsValid())
	{
		EndSkillFocus();
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		EndSkillFocus();
		return;
	}

	const double CurrentRealTime = World->GetRealTimeSeconds();
	const double RealDeltaTime = LastSkillFocusRealTime >= 0.0
		? FMath::Max(CurrentRealTime - LastSkillFocusRealTime, 0.0)
		: 0.0;
	LastSkillFocusRealTime = CurrentRealTime;
	SkillFocusElapsedTime += static_cast<float>(RealDeltaTime);

	const float BlendInDuration = FMath::Max(
		SkillFocusSettings.BlendInDuration,
		0.0f);
	const float HoldDuration = FMath::Max(
		SkillFocusSettings.HoldDuration,
		0.0f);
	const float BlendOutDuration = FMath::Max(
		SkillFocusSettings.BlendOutDuration,
		0.0f);
	const float BlendOutStartTime = BlendInDuration + HoldDuration;
	const float TotalDuration = BlendOutStartTime + BlendOutDuration;

	if (!bSkillFocusSlowMotionActive
		&& HoldDuration > 0.0f
		&& SkillFocusElapsedTime >= BlendInDuration
		&& SkillFocusElapsedTime < BlendOutStartTime)
	{
		BeginSkillFocusSlowMotion();
	}
	else if (bSkillFocusSlowMotionActive
		&& SkillFocusElapsedTime >= BlendOutStartTime)
	{
		EndSkillFocusSlowMotion();
	}

	if (SkillFocusElapsedTime >= TotalDuration)
	{
		EndSkillFocus();
		return;
	}

	float FocusAlpha = 1.0f;
	if (BlendInDuration > 0.0f
		&& SkillFocusElapsedTime < BlendInDuration)
	{
		FocusAlpha = SkillFocusElapsedTime / BlendInDuration;
	}
	else if (BlendOutDuration > 0.0f
		&& SkillFocusElapsedTime > BlendOutStartTime)
	{
		FocusAlpha = 1.0f
			- (SkillFocusElapsedTime - BlendOutStartTime) / BlendOutDuration;
	}

	FocusAlpha = FMath::SmoothStep(0.0f, 1.0f, FocusAlpha);
	CachedCameraComponent->SetFieldOfView(
		SkillFocusBaseFieldOfView
		- FMath::Max(SkillFocusSettings.FieldOfViewOffset, 0.0f) * FocusAlpha);
}

void UPBCombatCameraTrackingComponent::BeginSkillFocusSlowMotion()
{
	if (bSkillFocusSlowMotionActive)
	{
		return;
	}

	SkillFocusPreviousGlobalTimeDilation =
		UGameplayStatics::GetGlobalTimeDilation(this);
	const float AppliedTimeDilation = FMath::Min(
		SkillFocusPreviousGlobalTimeDilation,
		FMath::Clamp(SkillFocusSettings.GlobalTimeDilation, 0.01f, 1.0f));
	UGameplayStatics::SetGlobalTimeDilation(this, AppliedTimeDilation);
	bSkillFocusSlowMotionActive = true;

	UE_LOG(LogPBCombatCamera, Log,
		TEXT("Skill focus slow motion started. TimeDilation=%.2f"),
		AppliedTimeDilation);
}

void UPBCombatCameraTrackingComponent::EndSkillFocusSlowMotion()
{
	if (!bSkillFocusSlowMotionActive)
	{
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(
		this,
		SkillFocusPreviousGlobalTimeDilation);
	bSkillFocusSlowMotionActive = false;

	UE_LOG(LogPBCombatCamera, Log,
		TEXT("Skill focus slow motion ended. RestoredTimeDilation=%.2f"),
		SkillFocusPreviousGlobalTimeDilation);
}

void UPBCombatCameraTrackingComponent::EndSkillFocus()
{
	EndSkillFocusSlowMotion();

	if (bSkillFocusActive && CachedCameraComponent.IsValid())
	{
		CachedCameraComponent->SetFieldOfView(SkillFocusBaseFieldOfView);
	}

	bSkillFocusActive = false;
	SkillFocusBall.Reset();
	SkillFocusElapsedTime = 0.0f;
	LastSkillFocusRealTime = -1.0;
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
