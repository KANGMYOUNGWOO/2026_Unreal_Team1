#include "PBTurtleBoss.h"

#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossMoveArea.h"

APBTurtleBoss::APBTurtleBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	BossMovementType = EPBBossMovementType::Movable;

	TurtleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TurtleMesh"));
	TurtleMesh->SetupAttachment(GetRootComponent());

	FallingRockArea = CreateDefaultSubobject<UBoxComponent>(TEXT("FallingRockArea"));
	FallingRockArea->SetupAttachment(GetRootComponent());
	FallingRockArea->InitBoxExtent(FVector(700.0f, 700.0f, 100.0f));
	FallingRockArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FallingRockArea->SetGenerateOverlapEvents(false);
	FallingRockArea->SetHiddenInGame(true);
}

void APBTurtleBoss::BeginPlay()
{
	Super::BeginPlay();

	InitializeMoveArea();
	if (BossMoveArea)
	{
		FVector StartLocation = BossMoveArea->ClampLocation(GetActorLocation());
		StartLocation.Z = GetActorLocation().Z;
		SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (FallingRockArea)
	{
		FallingRockArea->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	SelectNextMoveTarget();
}

FVector APBTurtleBoss::GetRandomFallingRockLocation(float SpawnHeight) const
{
	if (!FallingRockArea)
	{
		return GetActorLocation() + FVector(0.0f, 0.0f, SpawnHeight);
	}

	const FVector AreaCenter = FallingRockArea->GetComponentLocation();
	const FVector AreaExtent = FallingRockArea->GetScaledBoxExtent().GetAbs();
	return FVector(
		FMath::FRandRange(AreaCenter.X - AreaExtent.X, AreaCenter.X + AreaExtent.X),
		FMath::FRandRange(AreaCenter.Y - AreaExtent.Y, AreaCenter.Y + AreaExtent.Y),
		AreaCenter.Z + SpawnHeight);
}

void APBTurtleBoss::PlayTurtleAnimation(UAnimationAsset* Animation)
{
	if (TurtleMesh && Animation)
	{
		TurtleMesh->PlayAnimation(Animation, false);
	}
}

void APBTurtleBoss::RestoreTurtleAnimationMode()
{
	if (TurtleMesh)
	{
		TurtleMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
}

void APBTurtleBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsIdleState())
	{
		MoveToTarget(DeltaTime);
	}
}

void APBTurtleBoss::StartIdleState_Implementation()
{
	Super::StartIdleState_Implementation();

	SelectNextMoveTarget();
}

void APBTurtleBoss::InitializeMoveArea()
{
	if (!BossMoveArea)
	{
		BossMoveArea = FindNearestMoveArea();
	}
}

void APBTurtleBoss::SelectNextMoveTarget()
{
	if (!BossMoveArea)
	{
		IsMoveTargetValid = false;
		return;
	}

	const FVector AreaCenter = BossMoveArea->GetAreaCenter();
	const FVector AreaExtent = BossMoveArea->GetAreaExtent().GetAbs();
	constexpr int32 MaxTargetSelectCount = 20;

	for (int32 TargetSelectIndex = 0; TargetSelectIndex < MaxTargetSelectCount; ++TargetSelectIndex)
	{
		MoveTargetLocation = AreaCenter + FVector(
			FMath::FRandRange(-AreaExtent.X, AreaExtent.X),
			FMath::FRandRange(-AreaExtent.Y, AreaExtent.Y),
			0.0f);
		MoveTargetLocation.Z = GetActorLocation().Z;

		if (FVector::DistSquared2D(GetActorLocation(), MoveTargetLocation) > FMath::Square(MoveAcceptanceRadius))
		{
			IsMoveTargetValid = true;
			return;
		}
	}

	IsMoveTargetValid = false;
}

void APBTurtleBoss::MoveToTarget(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	if (!IsMoveTargetValid)
	{
		SelectNextMoveTarget();
		if (!IsMoveTargetValid)
		{
			return;
		}
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector NextLocation = FMath::VInterpConstantTo(
		CurrentLocation,
		MoveTargetLocation,
		DeltaTime,
		MoveSpeed);

	SetActorLocation(NextLocation, false, nullptr, ETeleportType::None);

	if (FVector::DistSquared2D(NextLocation, MoveTargetLocation) <= FMath::Square(MoveAcceptanceRadius))
	{
		SelectNextMoveTarget();
	}
}

APBBossMoveArea* APBTurtleBoss::FindNearestMoveArea() const
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

	for (AActor* MoveAreaActor : MoveAreaActors)
	{
		APBBossMoveArea* MoveArea = Cast<APBBossMoveArea>(MoveAreaActor);
		if (!MoveArea)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(
			GetActorLocation(),
			MoveArea->GetAreaCenter());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestMoveArea = MoveArea;
			NearestDistanceSquared = DistanceSquared;
		}
	}

	return NearestMoveArea;
}
