#include "SnakeBoss.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ASnakeBoss::ASnakeBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	BossMovementType = EPBBossMovementType::Movable;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		SphereMesh = SphereMeshFinder.Object;
	}

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetRootComponent());
	ConfigureSnakeMeshComponent(HeadMesh);
	HeadMesh->SetRelativeScale3D(FVector(1.2f));
}

void ASnakeBoss::BeginPlay()
{
	Super::BeginPlay();

	InitializePatrolCenter();
	SetActorLocation(ClampLocationToPatrolArea(GetActorLocation()), false, nullptr, ETeleportType::TeleportPhysics);
	InitializeMoveDirection();

	IsPatrolTargetValid = false;
	SelectNextPatrolTarget();
	ResetSnakeTrail();
}

void ASnakeBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsIdleState())
	{
		RecoverCollapsedBody();
		MoveHead(DeltaTime);
		RecordHeadLocation();
		UpdateBodySegments();
		TrimHeadTrail();
	}

	DrawDebugSnake();
}

void ASnakeBoss::StartIdleState_Implementation()
{
	Super::StartIdleState_Implementation();

	RecoverCollapsedBody();
	IsPatrolTargetValid = false;
	SelectNextPatrolTarget();
}

void ASnakeBoss::FaceHeadDirection(const FVector& Direction)
{
	FVector LookDirection = Direction;
	LookDirection.Z = 0.0f;
	if (!LookDirection.Normalize())
	{
		return;
	}

	if (HeadMesh)
	{
		HeadMesh->SetWorldRotation(LookDirection.Rotation());
	}
}

void ASnakeBoss::CollapseBodyToHead()
{
	HeadTrail.Reset();
	SegmentTargetLocations.Reset();
	TotalTrailDistance = 0.0f;

	const FVector HeadLocation = GetActorLocation();
	for (UStaticMeshComponent* BodySegment : BodySegments)
	{
		if (!BodySegment)
		{
			continue;
		}

		BodySegment->SetWorldLocation(HeadLocation, false, nullptr, ETeleportType::TeleportPhysics);
		BodySegment->SetWorldRotation(GetActorRotation());
		SegmentTargetLocations.Add(HeadLocation);
	}

	HeadTrail.Add({HeadLocation, TotalTrailDistance});
	IsSnakeBodyCollapsed = true;
}

void ASnakeBoss::PullBodyToHead(float Alpha)
{
	const float PullAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	if (PullAlpha <= 0.0f)
	{
		return;
	}

	const FVector HeadLocation = GetActorLocation();
	SegmentTargetLocations.Reset();
	SegmentTargetLocations.Reserve(BodySegments.Num());

	for (int32 SegmentIndex = 0; SegmentIndex < BodySegments.Num(); ++SegmentIndex)
	{
		UStaticMeshComponent* BodySegment = BodySegments[SegmentIndex];
		if (!BodySegment)
		{
			continue;
		}

		const int32 TailOrderIndex = BodySegments.Num() - SegmentIndex - 1;
		const float SegmentStartAlpha = static_cast<float>(TailOrderIndex) * ChargePullDelayPerSegment;
		const float SegmentAlpha = FMath::Clamp((PullAlpha - SegmentStartAlpha) / FMath::Max(1.0f - SegmentStartAlpha, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
		const FVector SegmentLocation = FMath::Lerp(BodySegment->GetComponentLocation(), HeadLocation, SegmentAlpha);

		BodySegment->SetWorldLocation(SegmentLocation, false, nullptr, ETeleportType::TeleportPhysics);
		BodySegment->SetWorldRotation(GetActorRotation());
		SegmentTargetLocations.Add(SegmentLocation);
	}
}

void ASnakeBoss::InitializePatrolCenter()
{
	if (IsPatrolCenterInitialized)
	{
		return;
	}

	if (IsUseSpawnLocationAsPatrolCenter)
	{
		PatrolCenter = GetActorLocation();
	}

	IsPatrolCenterInitialized = true;
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

void ASnakeBoss::RecoverCollapsedBody()
{
	if (!IsSnakeBodyCollapsed)
	{
		return;
	}

	ResetSnakeTrail();
	IsSnakeBodyCollapsed = false;
}

void ASnakeBoss::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildSnakeSegments();
}

void ASnakeBoss::RebuildSnakeSegments()
{
	for (UStaticMeshComponent* BodySegment : BodySegments)
	{
		if (BodySegment)
		{
			BodySegment->DestroyComponent();
		}
	}

	BodySegments.Reset();
	BodySegments.Reserve(SegmentCount);

	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const FName SegmentName = *FString::Printf(TEXT("BodySegment_%02d"), SegmentIndex);
		UStaticMeshComponent* BodySegment = NewObject<UStaticMeshComponent>(this, SegmentName);
		if (!BodySegment)
		{
			continue;
		}

		BodySegment->SetupAttachment(GetRootComponent());
		ConfigureSnakeMeshComponent(BodySegment);
		BodySegment->SetRelativeScale3D(FVector(1.0f));
		BodySegment->RegisterComponent();
		AddInstanceComponent(BodySegment);
		BodySegments.Add(BodySegment);
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
			return;
		}
	}

	const FVector CurrentLocation = GetActorLocation();
	const float MoveDistance = MoveSpeed * DeltaTime;
	const float CurveDistance = FMath::Max(PatrolCurveDistance, PatrolAcceptanceRadius);
	PatrolCurveAlpha = FMath::Clamp(PatrolCurveAlpha + MoveDistance / CurveDistance, 0.0f, 1.0f);

	if (PatrolCurveAlpha >= 1.0f || FVector::Dist2D(CurrentLocation, PatrolTargetLocation) <= PatrolAcceptanceRadius)
	{
		SetActorLocation(PatrolTargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		SelectNextPatrolTarget();
		return;
	}

	const FVector NextLocation = ClampLocationToPatrolArea(GetPatrolCurveLocation(PatrolCurveAlpha));
	FVector MoveDirection = NextLocation - CurrentLocation;
	MoveDirection.Z = 0.0f;
	if (!MoveDirection.Normalize())
	{
		SelectNextPatrolTarget();
		return;
	}

	CurrentMoveDirection = FMath::VInterpNormalRotationTo(CurrentMoveDirection, MoveDirection, DeltaTime, TurnSpeed);
	SetActorLocation(NextLocation, false, nullptr, ETeleportType::TeleportPhysics);
	FaceMovementDirection(CurrentMoveDirection);
}

void ASnakeBoss::SelectNextPatrolTarget()
{
	PatrolStartLocation = ClampLocationToPatrolArea(GetActorLocation());
	const FVector PatrolExtent = PatrolAreaExtent.GetAbs();
	PatrolTargetLocation = PatrolCenter + FVector(
		FMath::FRandRange(-PatrolExtent.X, PatrolExtent.X),
		FMath::FRandRange(-PatrolExtent.Y, PatrolExtent.Y),
		0.0f);
	PatrolTargetLocation.Z = GetActorLocation().Z;

	const FVector TargetOffset = PatrolTargetLocation - PatrolStartLocation;
	FVector TargetDirection = TargetOffset;
	TargetDirection.Z = 0.0f;
	if (!TargetDirection.Normalize())
	{
		TargetDirection = CurrentMoveDirection;
	}

	const float ControlDistance = TargetOffset.Size2D() * PatrolCurveOffsetScale;
	PatrolCurveControlLocation = ClampLocationToPatrolArea(PatrolStartLocation + CurrentMoveDirection * ControlDistance);
	PatrolCurveEndControlLocation = ClampLocationToPatrolArea(PatrolTargetLocation - TargetDirection * ControlDistance);
	PatrolCurveAlpha = 0.0f;
	PatrolCurveDistance = CalculatePatrolCurveDistance();
	IsPatrolTargetValid = true;
}

void ASnakeBoss::ResetSnakeTrail()
{
	HeadTrail.Reset();
	SegmentTargetLocations.Reset();
	TotalTrailDistance = 0.0f;

	RecordHeadLocation();
	UpdateBodySegments();
}

FVector ASnakeBoss::ClampLocationToPatrolArea(const FVector& SourceLocation) const
{
	const FVector PatrolExtent = PatrolAreaExtent.GetAbs();
	FVector ClampedLocation = SourceLocation;
	ClampedLocation.X = FMath::Clamp(ClampedLocation.X, PatrolCenter.X - PatrolExtent.X, PatrolCenter.X + PatrolExtent.X);
	ClampedLocation.Y = FMath::Clamp(ClampedLocation.Y, PatrolCenter.Y - PatrolExtent.Y, PatrolCenter.Y + PatrolExtent.Y);
	ClampedLocation.Z = GetActorLocation().Z;
	return ClampedLocation;
}

FVector ASnakeBoss::GetPatrolCurveLocation(float Alpha) const
{
	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	const float InverseAlpha = 1.0f - ClampedAlpha;
	return PatrolStartLocation * InverseAlpha * InverseAlpha * InverseAlpha
		+ PatrolCurveControlLocation * 3.0f * InverseAlpha * InverseAlpha * ClampedAlpha
		+ PatrolCurveEndControlLocation * 3.0f * InverseAlpha * ClampedAlpha * ClampedAlpha
		+ PatrolTargetLocation * ClampedAlpha * ClampedAlpha * ClampedAlpha;
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

void ASnakeBoss::FaceMovementDirection(const FVector& Direction)
{
	FVector MoveDirection = Direction;
	MoveDirection.Z = 0.0f;
	if (!MoveDirection.Normalize())
	{
		return;
	}

	CurrentMoveDirection = MoveDirection;
	const FRotator MoveRotation = CurrentMoveDirection.Rotation();
	SetActorRotation(MoveRotation);
	if (HeadMesh)
	{
		HeadMesh->SetWorldRotation(MoveRotation);
	}
}

void ASnakeBoss::RecordHeadLocation()
{
	const FVector HeadLocation = GetActorLocation();
	if (HeadTrail.Num() > 0)
	{
		const float MoveDistance = FVector::Dist2D(HeadTrail.Last().Location, HeadLocation);
		if (MoveDistance <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		TotalTrailDistance += MoveDistance;
	}

	HeadTrail.Add({HeadLocation, TotalTrailDistance});
}

void ASnakeBoss::UpdateBodySegments()
{
	SegmentTargetLocations.Reset();
	SegmentTargetLocations.Reserve(BodySegments.Num());

	for (int32 SegmentIndex = 0; SegmentIndex < BodySegments.Num(); ++SegmentIndex)
	{
		UStaticMeshComponent* BodySegment = BodySegments[SegmentIndex];
		if (!BodySegment)
		{
			continue;
		}

		FVector TargetLocation = GetActorLocation();
		const float TargetDistance = TotalTrailDistance - SegmentSpacing * static_cast<float>(SegmentIndex + 1);
		FindTrailLocationAtDistance(TargetDistance, TargetLocation);

		BodySegment->SetWorldLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		FVector ForwardLocation = TargetLocation;
		FindTrailLocationAtDistance(TargetDistance + SegmentSpacing * 0.5f, ForwardLocation);

		FVector SegmentDirection = ForwardLocation - TargetLocation;
		SegmentDirection.Z = 0.0f;
		if (SegmentDirection.Normalize())
		{
			BodySegment->SetWorldRotation(SegmentDirection.Rotation());
		}

		SegmentTargetLocations.Add(TargetLocation);
	}
}

void ASnakeBoss::DrawDebugSnake() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!IsDrawDebugSnake)
	{
		return;
	}

	for (int32 TrailIndex = 1; TrailIndex < HeadTrail.Num(); ++TrailIndex)
	{
		DrawDebugLine(
			World,
			HeadTrail[TrailIndex - 1].Location,
			HeadTrail[TrailIndex].Location,
			FColor::Green,
			false,
			0.0f,
			0,
			2.0f);
	}

	for (const FVector& SegmentTargetLocation : SegmentTargetLocations)
	{
		DrawDebugSphere(
			World,
			SegmentTargetLocation,
			24.0f,
			12,
			FColor::Yellow,
			false,
			0.0f,
			0,
			2.0f);
	}

	const FVector PatrolExtent = PatrolAreaExtent.GetAbs();
	DrawDebugBox(
		World,
		PatrolCenter,
		FVector(PatrolExtent.X, PatrolExtent.Y, 10.0f),
		FColor::Red,
		false,
		0.0f,
		0,
		2.0f);

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

void ASnakeBoss::TrimHeadTrail()
{
	const float KeepDistance = SegmentSpacing * static_cast<float>(SegmentCount + 2);
	const float OldestDistance = TotalTrailDistance - KeepDistance;

	while (HeadTrail.Num() > 1 && HeadTrail[1].Distance < OldestDistance)
	{
		HeadTrail.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

bool ASnakeBoss::FindTrailLocationAtDistance(float TargetDistance, FVector& OutLocation) const
{
	if (HeadTrail.Num() == 0)
	{
		return false;
	}

	if (TargetDistance <= HeadTrail[0].Distance)
	{
		OutLocation = HeadTrail[0].Location;
		return true;
	}

	for (int32 TrailIndex = 1; TrailIndex < HeadTrail.Num(); ++TrailIndex)
	{
		const FPBSnakeBossTrailSample& PreviousSample = HeadTrail[TrailIndex - 1];
		const FPBSnakeBossTrailSample& CurrentSample = HeadTrail[TrailIndex];
		if (CurrentSample.Distance < TargetDistance)
		{
			continue;
		}

		const float SegmentDistance = CurrentSample.Distance - PreviousSample.Distance;
		if (FMath::IsNearlyZero(SegmentDistance))
		{
			OutLocation = CurrentSample.Location;
			return true;
		}

		const float SegmentAlpha = FMath::Clamp(
			(TargetDistance - PreviousSample.Distance) / SegmentDistance,
			0.0f,
			1.0f);
		OutLocation = FMath::Lerp(PreviousSample.Location, CurrentSample.Location, SegmentAlpha);
		return true;
	}

	OutLocation = HeadTrail.Last().Location;
	return true;
}

void ASnakeBoss::StartSnakePattern()
{
}

void ASnakeBoss::StopSnakePattern()
{
}

void ASnakeBoss::EnterSnakeGroggy()
{
}

void ASnakeBoss::ExitSnakeGroggy()
{
}

void ASnakeBoss::ConfigureSnakeMeshComponent(UStaticMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetStaticMesh(SphereMesh);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetMobility(EComponentMobility::Movable);
}
