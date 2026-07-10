#include "PBBossChargeTelegraph.h"

#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"

APBBossChargeTelegraph::APBBossChargeTelegraph()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APBBossChargeTelegraph::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	static_cast<void>(DeltaSeconds);
	UpdateTrackedPinballTransform();
}

void APBBossChargeTelegraph::InitTelegraph(float InDurationSeconds, const FVector& InScale)
{
	Super::InitTelegraph(InDurationSeconds, InScale);

	ChargeStartLocation = GetActorLocation();
	CurrentDirection = GetActorForwardVector();
	CurrentDirection.Z = 0.0f;
	CurrentDirection = CurrentDirection.GetSafeNormal();
	if (CurrentDirection.IsNearlyZero())
	{
		CurrentDirection = FVector::ForwardVector;
	}

	UpdateTrackedPinballTransform();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TelegraphDurationTimerHandle,
			this,
			&APBBossChargeTelegraph::HandleTelegraphDurationFinished,
			FMath::Max(0.001f, InDurationSeconds),
			false);
	}
}

void APBBossChargeTelegraph::DestroyTelegraph()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TelegraphDurationTimerHandle);
	}

	Super::DestroyTelegraph();
}

void APBBossChargeTelegraph::InitChargeTelegraph(
	float InDurationSeconds,
	const FVector& StartLocation,
	const FVector& Direction,
	float Length,
	const FVector& InScale)
{
	const FVector AppliedScale = GetActorScale3D() * InScale;
	ChargeStartLocation = StartLocation;
	CurrentDirection = Direction;
	CurrentLength = FMath::Max(0.0f, Length);
	UpdateChargeTelegraphTransform(StartLocation, Direction, Length);
	InitTelegraph(InDurationSeconds, AppliedScale);
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

	SetActorLocation(StartLocation);
	SetActorRotation(SafeDirection.Rotation());
	UpdateVisualComponentOffsets(SafeLength);

	ChargeStartLocation = StartLocation;
	CurrentDirection = SafeDirection;
	CurrentLength = SafeLength;
	CurrentTargetLocation = StartLocation + SafeDirection * SafeLength;
}

FVector APBBossChargeTelegraph::GetCurrentTargetLocation() const
{
	return CurrentTargetLocation;
}

FVector APBBossChargeTelegraph::GetCurrentDirection() const
{
	return CurrentDirection;
}

void APBBossChargeTelegraph::HandleTelegraphDurationFinished()
{
	UpdateTrackedPinballTransform();
	OnChargeTelegraphFinished.Broadcast(CurrentTargetLocation, CurrentDirection);
	DestroyTelegraph();
}

void APBBossChargeTelegraph::UpdateTrackedPinballTransform()
{
	AActor* PinballActor = FindPinballActor();
	if (!PinballActor)
	{
		UpdateChargeTelegraphTransform(ChargeStartLocation, CurrentDirection, CurrentLength);
		return;
	}

	const FVector TargetLocation = PinballActor->GetActorLocation();
	const FVector Direction = CalculateDirectionToTarget(TargetLocation);
	const float Length = FVector::Dist2D(ChargeStartLocation, TargetLocation);
	UpdateChargeTelegraphTransform(ChargeStartLocation, Direction, Length);
	CurrentTargetLocation = TargetLocation;
}

void APBBossChargeTelegraph::UpdateVisualComponentOffsets(float Length)
{
	if (!IsVisualOffsetToPathCenter)
	{
		return;
	}

	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (!SceneComponent || SceneComponent == GetRootComponent())
		{
			continue;
		}

		const TObjectKey<USceneComponent> SceneComponentKey(SceneComponent);
		const FVector InitialRelativeLocation = InitialRelativeLocationMap.FindOrAdd(
			SceneComponentKey,
			SceneComponent->GetRelativeLocation());

		SceneComponent->SetRelativeLocation(InitialRelativeLocation + FVector(Length * 0.5f, 0.0f, 0.0f));
	}
}

AActor* APBBossChargeTelegraph::FindPinballActor() const
{
	const UWorld* World = GetWorld();
	return World ? UGameplayStatics::GetActorOfClass(World, APBBallBase::StaticClass()) : nullptr;
}

FVector APBBossChargeTelegraph::CalculateDirectionToTarget(const FVector& TargetLocation) const
{
	FVector Direction = TargetLocation - ChargeStartLocation;
	Direction.Z = 0.0f;
	Direction = Direction.GetSafeNormal();
	return Direction.IsNearlyZero() ? FVector::ForwardVector : Direction;
}
