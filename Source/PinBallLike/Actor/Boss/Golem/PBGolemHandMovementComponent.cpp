#include "PBGolemHandMovementComponent.h"

#include "GameFramework/Actor.h"

UPBGolemHandMovementComponent::UPBGolemHandMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPBGolemHandMovementComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (MovementMode == EPBGolemHandMovementMode::Autonomous && !IsMovingValue)
	{
		UpdateAutonomousMove(DeltaTime);
		return;
	}

	UpdateCurrentMove(DeltaTime);
}

void UPBGolemHandMovementComponent::UpdateCurrentMove(float DeltaTime)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !IsMovingValue)
	{
		return;
	}

	if (MoveDuration <= 0.0f)
	{
		OwnerActor->SetActorLocation(TargetLocation);
		FinishMove();
		return;
	}

	ElapsedMoveTime += DeltaTime;
	const float Alpha = FMath::Clamp(ElapsedMoveTime / MoveDuration, 0.0f, 1.0f);
	OwnerActor->SetActorLocation(FMath::Lerp(StartLocation, TargetLocation, Alpha));

	if (Alpha >= 1.0f)
	{
		FinishMove();
	}
}

void UPBGolemHandMovementComponent::InitializeHandMovement(AActor* NewBossActor, FVector NewDefaultOffset)
{
	BossActor = NewBossActor;
	DefaultOffset = NewDefaultOffset;

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->SetActorLocation(ConvertOffsetToWorldLocation(DefaultOffset));
	}
}

void UPBGolemHandMovementComponent::MoveToOffset(FVector TargetOffset, float Duration)
{
	StartMove(TargetOffset, Duration, EPBGolemHandMovementMode::Command);
}

void UPBGolemHandMovementComponent::MoveToWorldLocation(FVector TargetWorldLocation, float Duration)
{
	StartMoveToWorldLocation(TargetWorldLocation, Duration, EPBGolemHandMovementMode::Command);
}

void UPBGolemHandMovementComponent::ReturnToDefaultOffset(float Duration)
{
	MoveToOffset(DefaultOffset, Duration);
}

void UPBGolemHandMovementComponent::StartAutonomousMove()
{
	MovementMode = EPBGolemHandMovementMode::Autonomous;
	IsMovingValue = false;
	AutonomousWaitTime = 0.0f;
	SetComponentTickEnabled(true);
}

void UPBGolemHandMovementComponent::StopAutonomousMove()
{
	if (MovementMode != EPBGolemHandMovementMode::Autonomous)
	{
		return;
	}

	StopMove();
}

void UPBGolemHandMovementComponent::StopMove()
{
	IsMovingValue = false;
	MovementMode = EPBGolemHandMovementMode::Idle;
	SetComponentTickEnabled(false);
}

bool UPBGolemHandMovementComponent::IsMoving() const
{
	return IsMovingValue;
}

EPBGolemHandMovementMode UPBGolemHandMovementComponent::GetMovementMode() const
{
	return MovementMode;
}

FVector UPBGolemHandMovementComponent::GetDefaultOffset() const
{
	return DefaultOffset;
}

FVector UPBGolemHandMovementComponent::ConvertOffsetToWorldLocation(FVector Offset) const
{
	return BossActor ? BossActor->GetActorTransform().TransformPosition(Offset) : Offset;
}

void UPBGolemHandMovementComponent::StartMove(
	FVector TargetOffset,
	float Duration,
	EPBGolemHandMovementMode NewMovementMode)
{
	StartMoveToWorldLocation(ConvertOffsetToWorldLocation(TargetOffset), Duration, NewMovementMode);
}

void UPBGolemHandMovementComponent::StartMoveToWorldLocation(
	FVector TargetWorldLocation,
	float Duration,
	EPBGolemHandMovementMode NewMovementMode)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	StartLocation = OwnerActor->GetActorLocation();
	TargetLocation = TargetWorldLocation;
	MoveDuration = FMath::Max(0.0f, Duration);
	ElapsedMoveTime = 0.0f;
	MovementMode = NewMovementMode;
	IsMovingValue = true;
	SetComponentTickEnabled(true);
}

void UPBGolemHandMovementComponent::UpdateAutonomousMove(float DeltaTime)
{
	AutonomousWaitTime -= DeltaTime;
	if (AutonomousWaitTime > 0.0f)
	{
		return;
	}

	SelectNextAutonomousMove();
}

void UPBGolemHandMovementComponent::SelectNextAutonomousMove()
{
	const FVector2D RandomDirection = FMath::RandPointInCircle(FMath::Max(0.0f, AutonomousMoveRadius));
	const FVector TargetOffset = DefaultOffset + FVector(RandomDirection.X, RandomDirection.Y, 0.0f);
	const float MinDuration = FMath::Max(0.1f, AutonomousMoveDurationMin);
	const float MaxDuration = FMath::Max(MinDuration, AutonomousMoveDurationMax);
	const float Duration = FMath::FRandRange(MinDuration, MaxDuration);

	StartMove(TargetOffset, Duration, EPBGolemHandMovementMode::Autonomous);
}

void UPBGolemHandMovementComponent::FinishMove()
{
	IsMovingValue = false;

	if (MovementMode == EPBGolemHandMovementMode::Autonomous)
	{
		const float MinWaitTime = FMath::Max(0.0f, AutonomousWaitTimeMin);
		const float MaxWaitTime = FMath::Max(MinWaitTime, AutonomousWaitTimeMax);
		AutonomousWaitTime = FMath::FRandRange(MinWaitTime, MaxWaitTime);
		OnMoveFinished.Broadcast();
		return;
	}

	MovementMode = EPBGolemHandMovementMode::Idle;
	SetComponentTickEnabled(false);
	OnMoveFinished.Broadcast();
}
