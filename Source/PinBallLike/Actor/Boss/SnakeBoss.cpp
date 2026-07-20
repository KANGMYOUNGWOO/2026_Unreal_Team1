#include "SnakeBoss.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossMoveArea.h"

namespace
{
	FVector CalculateCubicBezierLocation(
		const FVector& StartLocation,
		const FVector& FirstControlLocation,
		const FVector& SecondControlLocation,
		const FVector& EndLocation,
		float Alpha)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		const float InverseAlpha = 1.0f - ClampedAlpha;

		return StartLocation * InverseAlpha * InverseAlpha * InverseAlpha
			+ FirstControlLocation * 3.0f * InverseAlpha * InverseAlpha * ClampedAlpha
			+ SecondControlLocation * 3.0f * InverseAlpha * ClampedAlpha * ClampedAlpha
			+ EndLocation * ClampedAlpha * ClampedAlpha * ClampedAlpha;
	}
}

ASnakeBoss::ASnakeBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	SnakeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SnakeMesh"));
	SnakeMesh->SetupAttachment(GetRootComponent());
	SnakeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SnakeMesh->SetCollisionResponseToAllChannels(ECR_Block);
	SnakeMesh->SetGenerateOverlapEvents(true);
	SnakeMesh->SetNotifyRigidBodyCollision(true);
	SnakeMesh->SetMobility(EComponentMobility::Movable);
}

void ASnakeBoss::BeginPlay()
{
	Super::BeginPlay();

	InitializeMoveArea();
	SetActorLocation(ClampLocationToPatrolArea(GetActorLocation()), false, nullptr, ETeleportType::TeleportPhysics);
	InitializeMoveDirection();
	ResetSnakePath();

	IsPatrolTargetValid = false;
	SelectNextPatrolTarget();
}

void ASnakeBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsIdleState())
	{
		MoveHead(DeltaTime);
	}
	else
	{
		UpdateSnakeAnimationData(DeltaTime, GetActorLocation(), GetActorLocation());
	}

	DrawDebugSnake();
}

void ASnakeBoss::StartIdleState_Implementation()
{
	Super::StartIdleState_Implementation();

	IsPatrolTargetValid = false;
	SelectNextPatrolTarget();
}

void ASnakeBoss::SetSnakeProjectilePose(bool IsActive, float BlendAlpha)
{
	IsSnakeProjectilePoseActiveValue = IsActive;
	SnakeProjectilePoseAlpha = IsActive ? FMath::Clamp(BlendAlpha, 0.0f, 1.0f) : 0.0f;
}

void ASnakeBoss::SetSnakePinballCollisionEnabled(bool IsEnabled)
{
	if (IsEnabled)
	{
		if (!IsSnakePinballCollisionDisabled)
		{
			return;
		}

		if (CollisionSphere)
		{
			CollisionSphere->SetCollisionResponseToChannel(
				ECC_PhysicsBody,
				CachedCollisionSpherePhysicsBodyResponse);
		}

		if (SnakeMesh)
		{
			SnakeMesh->SetCollisionResponseToChannel(
				ECC_PhysicsBody,
				CachedSnakeMeshPhysicsBodyResponse);
		}

		IsSnakePinballCollisionDisabled = false;
		return;
	}

	if (IsSnakePinballCollisionDisabled)
	{
		return;
	}

	if (CollisionSphere)
	{
		CachedCollisionSpherePhysicsBodyResponse = CollisionSphere->GetCollisionResponseToChannel(ECC_PhysicsBody);
		CollisionSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	}

	if (SnakeMesh)
	{
		CachedSnakeMeshPhysicsBodyResponse = SnakeMesh->GetCollisionResponseToChannel(ECC_PhysicsBody);
		SnakeMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	}

	IsSnakePinballCollisionDisabled = true;
}

void ASnakeBoss::UpdateSnakeChargeMovement(
	float DeltaTime,
	const FVector& PreviousLocation,
	const FVector& NextLocation)
{
	UpdateSnakeAnimationData(DeltaTime, NextLocation, PreviousLocation);
	RecordSnakePathLocation(NextLocation);
	UpdateSnakeSplinePoints();
}

float ASnakeBoss::GetSnakeAnimationSpeed() const
{
	return SnakeAnimationSpeed;
}

float ASnakeBoss::GetSnakeAnimationTurnAmount() const
{
	return SnakeAnimationTurnAmount;
}

float ASnakeBoss::GetSnakeAnimationMovePhase() const
{
	return SnakeAnimationMovePhase;
}

bool ASnakeBoss::IsSnakeAnimationMoving() const
{
	return IsSnakeAnimationMoveActive;
}

const TArray<FVector>& ASnakeBoss::GetSnakeSplinePoints() const
{
	return SnakeSplinePoints;
}

bool ASnakeBoss::IsSnakeProjectilePoseActive() const
{
	return IsSnakeProjectilePoseActiveValue;
}

float ASnakeBoss::GetSnakeProjectilePoseAlpha() const
{
	return SnakeProjectilePoseAlpha;
}

void ASnakeBoss::InitializeMoveArea()
{
	if (!BossMoveArea)
	{
		BossMoveArea = FindNearestMoveArea();
	}
}

void ASnakeBoss::InitializeMoveDirection()
{
	CurrentMoveDirection = GetActorForwardVector();
	CurrentMoveDirection.Z = 0.0f;
	if (!CurrentMoveDirection.Normalize())
	{
		CurrentMoveDirection = FVector::ForwardVector;
	}
}

void ASnakeBoss::MoveHead(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	if (!IsPatrolTargetValid)
	{
		SelectNextPatrolTarget();
		if (!IsPatrolTargetValid)
		{
			UpdateSnakeAnimationData(DeltaTime, GetActorLocation(), GetActorLocation());
			return;
		}
	}

	const FVector CurrentLocation = GetActorLocation();
	const float MoveDistance = MoveSpeed * DeltaTime;
	PatrolCurveAlpha = CalculateNextPatrolCurveAlpha(MoveDistance);
	const FVector NextLocation = ClampLocationToPatrolArea(GetPatrolCurveLocation(PatrolCurveAlpha));
	FVector MoveDirection = NextLocation - CurrentLocation;
	MoveDirection.Z = 0.0f;
	if (!MoveDirection.Normalize())
	{
		if (PatrolCurveAlpha >= 1.0f)
		{
			SelectNextPatrolTarget();
		}
		return;
	}

	CurrentMoveDirection = FMath::VInterpNormalRotationTo(CurrentMoveDirection, MoveDirection, DeltaTime, TurnSpeed);
	SetActorLocation(NextLocation, false, nullptr, ETeleportType::None);
	FaceMovementDirection(CurrentMoveDirection);
	UpdateSnakeAnimationData(DeltaTime, NextLocation, CurrentLocation);
	RecordSnakePathLocation(NextLocation);
	UpdateSnakeSplinePoints();

	if (PatrolCurveAlpha >= 1.0f)
	{
		SelectNextPatrolTarget();
	}
}

void ASnakeBoss::UpdateSnakeAnimationData(float DeltaTime, const FVector& NextLocation, const FVector& PreviousLocation)
{
	if (DeltaTime <= 0.0f)
	{
		SnakeAnimationSpeed = 0.0f;
		IsSnakeAnimationMoveActive = false;
		return;
	}

	FVector DeltaLocation = NextLocation - PreviousLocation;
	DeltaLocation.Z = 0.0f;

	SnakeAnimationSpeed = DeltaLocation.Size() / DeltaTime;
	IsSnakeAnimationMoveActive = SnakeAnimationSpeed > KINDA_SMALL_NUMBER;

	FVector MoveDirection = DeltaLocation;
	if (MoveDirection.Normalize())
	{
		const FVector ForwardDirection = GetActorForwardVector().GetSafeNormal2D();
		const FVector CrossProduct = FVector::CrossProduct(ForwardDirection, MoveDirection);
		const float DotProduct = FMath::Clamp(FVector::DotProduct(ForwardDirection, MoveDirection), -1.0f, 1.0f);
		SnakeAnimationTurnAmount = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct.Z, DotProduct));
	}
	else
	{
		SnakeAnimationTurnAmount = 0.0f;
	}

	if (IsSnakeAnimationMoveActive)
	{
		SnakeAnimationMovePhase = FMath::Fmod(SnakeAnimationMovePhase + SnakeAnimationSpeed * DeltaTime, 360.0f);
	}
}

void ASnakeBoss::ResetSnakePath()
{
	SnakePathSamples.Reset();
	SnakePathTotalDistance = 0.0f;
	RecordSnakePathLocation(GetActorLocation());
	UpdateSnakeSplinePoints();
}

void ASnakeBoss::RecordSnakePathLocation(const FVector& Location)
{
	FVector PathLocation = Location;
	PathLocation.Z = GetActorLocation().Z;

	if (SnakePathSamples.Num() > 0)
	{
		const float MoveDistance = FVector::Dist2D(SnakePathSamples.Last().Location, PathLocation);
		if (MoveDistance <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		SnakePathTotalDistance += MoveDistance;
	}

	FPBSnakePathSample PathSample;
	PathSample.Location = PathLocation;
	PathSample.Distance = SnakePathTotalDistance;
	SnakePathSamples.Add(PathSample);

	TrimSnakePath();
}

void ASnakeBoss::UpdateSnakeSplinePoints()
{
	const int32 SplinePointCount = FMath::Max(SnakeSplinePointCount, 2);
	const float SplineLength = FMath::Max(SnakeSplineLength, 1.0f);
	const float PointSpacing = SplineLength / static_cast<float>(SplinePointCount - 1);
	const FTransform MeshTransform = SnakeMesh ? SnakeMesh->GetComponentTransform() : GetActorTransform();

	SnakeSplinePoints.Reset();
	SnakeSplinePoints.Reserve(SplinePointCount);

	for (int32 PointIndex = 0; PointIndex < SplinePointCount; ++PointIndex)
	{
		const float TargetDistance = SnakePathTotalDistance - PointSpacing * static_cast<float>(PointIndex);
		FVector PointLocation = GetActorLocation();
		FindSnakePathLocationAtDistance(TargetDistance, PointLocation);
		SnakeSplinePoints.Add(MeshTransform.InverseTransformPosition(PointLocation));
	}
}

void ASnakeBoss::TrimSnakePath()
{
	const float KeepDistance = FMath::Max(SnakeSplineLength, 1.0f) + MoveSpeed;
	const float OldestDistance = SnakePathTotalDistance - KeepDistance;

	while (SnakePathSamples.Num() > 1 && SnakePathSamples[1].Distance < OldestDistance)
	{
		SnakePathSamples.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

bool ASnakeBoss::FindSnakePathLocationAtDistance(float Distance, FVector& OutLocation) const
{
	if (SnakePathSamples.Num() == 0)
	{
		return false;
	}

	if (Distance <= SnakePathSamples[0].Distance)
	{
		OutLocation = SnakePathSamples[0].Location;
		return true;
	}

	for (int32 SampleIndex = 1; SampleIndex < SnakePathSamples.Num(); ++SampleIndex)
	{
		const FPBSnakePathSample& PreviousSample = SnakePathSamples[SampleIndex - 1];
		const FPBSnakePathSample& CurrentSample = SnakePathSamples[SampleIndex];
		if (CurrentSample.Distance < Distance)
		{
			continue;
		}

		const float SampleDistance = CurrentSample.Distance - PreviousSample.Distance;
		if (FMath::IsNearlyZero(SampleDistance))
		{
			OutLocation = CurrentSample.Location;
			return true;
		}

		const float SampleAlpha = FMath::Clamp((Distance - PreviousSample.Distance) / SampleDistance, 0.0f, 1.0f);
		OutLocation = FMath::Lerp(PreviousSample.Location, CurrentSample.Location, SampleAlpha);
		return true;
	}

	OutLocation = SnakePathSamples.Last().Location;
	return true;
}

void ASnakeBoss::SelectNextPatrolTarget()
{
	if (!BossMoveArea)
	{
		IsPatrolTargetValid = false;
		return;
	}

	PatrolStartLocation = ClampLocationToPatrolArea(GetActorLocation());
	const FVector PatrolCenter = BossMoveArea->GetAreaCenter();
	const FVector PatrolExtent = BossMoveArea->GetAreaExtent().GetAbs();
	constexpr int32 MaxTargetSelectCount = 20;
	bool IsTargetSelected = false;

	for (int32 TargetSelectIndex = 0; TargetSelectIndex < MaxTargetSelectCount; ++TargetSelectIndex)
	{
		const FVector RandomLocation = PatrolCenter + FVector(
			FMath::FRandRange(-PatrolExtent.X, PatrolExtent.X),
			FMath::FRandRange(-PatrolExtent.Y, PatrolExtent.Y),
			0.0f);

		PatrolTargetLocation = RandomLocation;
		PatrolTargetLocation.Z = GetActorLocation().Z;

		FVector CandidateDirection = PatrolTargetLocation - PatrolStartLocation;
		CandidateDirection.Z = 0.0f;
		if (CandidateDirection.SizeSquared2D() <= FMath::Square(PatrolAcceptanceRadius))
		{
			continue;
		}

		if (!CandidateDirection.Normalize())
		{
			continue;
		}

		const float ForwardDot = FVector::DotProduct(CurrentMoveDirection, CandidateDirection);
		if (!IsInsideHeadExcludedArea(PatrolTargetLocation) && ForwardDot >= PatrolTargetForwardDot)
		{
			IsTargetSelected = true;
			break;
		}
	}

	if (!IsTargetSelected)
	{
		IsPatrolTargetValid = false;
		return;
	}

	const FVector TargetOffset = PatrolTargetLocation - PatrolStartLocation;
	FVector TargetDirection = TargetOffset;
	TargetDirection.Z = 0.0f;
	if (!TargetDirection.Normalize())
	{
		TargetDirection = CurrentMoveDirection;
	}

	const float ControlDistance = FMath::Max(TargetOffset.Size2D() * PatrolCurveOffsetScale, PatrolMinCurveControlDistance);
	PatrolCurveControlLocation = ClampLocationToPatrolArea(PatrolStartLocation + CurrentMoveDirection * ControlDistance);
	PatrolCurveEndControlLocation = ClampLocationToPatrolArea(PatrolTargetLocation - TargetDirection * ControlDistance);
	PatrolCurveAlpha = 0.0f;
	PatrolCurveDistance = CalculatePatrolCurveDistance();
	IsPatrolTargetValid = true;
}

FVector ASnakeBoss::ClampLocationToPatrolArea(const FVector& SourceLocation) const
{
	if (!BossMoveArea)
	{
		FVector Location = SourceLocation;
		Location.Z = GetActorLocation().Z;
		return Location;
	}

	FVector ClampedLocation = BossMoveArea->ClampLocation(SourceLocation);
	ClampedLocation.Z = GetActorLocation().Z;
	return ClampedLocation;
}

APBBossMoveArea* ASnakeBoss::FindNearestMoveArea() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> MoveAreaActors;
	UGameplayStatics::GetAllActorsOfClass(World, APBBossMoveArea::StaticClass(), MoveAreaActors);

	APBBossMoveArea* NearestMoveArea = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();
	const FVector BossLocation = GetActorLocation();

	for (AActor* MoveAreaActor : MoveAreaActors)
	{
		APBBossMoveArea* MoveArea = Cast<APBBossMoveArea>(MoveAreaActor);
		if (!MoveArea)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(BossLocation, MoveArea->GetAreaCenter());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestMoveArea = MoveArea;
			NearestDistanceSquared = DistanceSquared;
		}
	}

	return NearestMoveArea;
}

bool ASnakeBoss::IsInsideHeadExcludedArea(const FVector& SourceLocation) const
{
	if (HeadExcludeRadius <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared2D(GetActorLocation(), SourceLocation) <= FMath::Square(HeadExcludeRadius);
}

FVector ASnakeBoss::GetPatrolCurveLocation(float Alpha) const
{
	return CalculateCubicBezierLocation(
		PatrolStartLocation,
		PatrolCurveControlLocation,
		PatrolCurveEndControlLocation,
		PatrolTargetLocation,
		Alpha);
}

float ASnakeBoss::CalculatePatrolCurveDistance() const
{
	constexpr int32 SampleCount = 12;
	float CurveDistance = 0.0f;
	FVector PreviousLocation = PatrolStartLocation;

	for (int32 SampleIndex = 1; SampleIndex <= SampleCount; ++SampleIndex)
	{
		const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(SampleCount);
		const FVector CurrentLocation = GetPatrolCurveLocation(Alpha);
		CurveDistance += FVector::Dist2D(PreviousLocation, CurrentLocation);
		PreviousLocation = CurrentLocation;
	}

	return CurveDistance;
}

float ASnakeBoss::CalculateNextPatrolCurveAlpha(float MoveDistance) const
{
	if (MoveDistance <= 0.0f || PatrolCurveAlpha >= 1.0f || PatrolCurveDistance <= KINDA_SMALL_NUMBER)
	{
		return PatrolCurveAlpha;
	}

	const FVector CurrentCurveLocation = GetPatrolCurveLocation(PatrolCurveAlpha);
	if (FVector::Dist2D(CurrentCurveLocation, PatrolTargetLocation) <= MoveDistance)
	{
		return 1.0f;
	}

	float MinimumAlpha = PatrolCurveAlpha;
	float MaximumAlpha = 1.0f;
	constexpr int32 SearchIterationCount = 10;

	for (int32 SearchIndex = 0; SearchIndex < SearchIterationCount; ++SearchIndex)
	{
		const float CandidateAlpha = (MinimumAlpha + MaximumAlpha) * 0.5f;
		const float CandidateDistance = FVector::Dist2D(
			CurrentCurveLocation,
			GetPatrolCurveLocation(CandidateAlpha));

		if (CandidateDistance < MoveDistance)
		{
			MinimumAlpha = CandidateAlpha;
		}
		else
		{
			MaximumAlpha = CandidateAlpha;
		}
	}

	return MaximumAlpha;
}

void ASnakeBoss::FaceMovementDirection(const FVector& Direction)
{
	FVector MoveDirection = Direction;
	MoveDirection.Z = 0.0f;
	if (!MoveDirection.Normalize())
	{
		return;
	}

	CurrentMoveDirection = MoveDirection;
	SetActorRotation(CurrentMoveDirection.Rotation());
}

void ASnakeBoss::DrawDebugSnake() const
{
	UWorld* World = GetWorld();
	if (!World || !IsDrawDebugSnake)
	{
		return;
	}

	const FTransform MeshTransform = SnakeMesh ? SnakeMesh->GetComponentTransform() : GetActorTransform();
	for (const FVector& SnakeSplinePoint : SnakeSplinePoints)
	{
		DrawDebugSphere(
			World,
			MeshTransform.TransformPosition(SnakeSplinePoint),
			24.0f,
			10,
			FColor::Yellow,
			false,
			0.0f,
			0,
			2.0f);
	}

	if (BossMoveArea)
	{
		const FVector PatrolCenter = BossMoveArea->GetAreaCenter();
		const FVector PatrolExtent = BossMoveArea->GetAreaExtent().GetAbs();
		DrawDebugBox(
			World,
			PatrolCenter,
			FVector(PatrolExtent.X, PatrolExtent.Y, 10.0f),
			FColor::Red,
			false,
			0.0f,
			0,
			2.0f);
	}

	if (IsPatrolTargetValid)
	{
		DrawDebugSphere(
			World,
			PatrolTargetLocation,
			35.0f,
			12,
			FColor::Red,
			false,
			0.0f,
			0,
			2.0f);
		DrawDebugLine(
			World,
			GetActorLocation(),
			PatrolTargetLocation,
			FColor::Red,
			false,
			0.0f,
			0,
			2.0f);
	}
}
