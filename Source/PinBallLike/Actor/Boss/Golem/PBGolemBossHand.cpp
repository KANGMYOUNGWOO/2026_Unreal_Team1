#include "PBGolemBossHand.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PBGolemBoss.h"
#include "PBGolemHandMovementComponent.h"

APBGolemBossHand::APBGolemBossHand()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandMesh"));
	HandMesh->SetupAttachment(SceneRoot);
	HandMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HandMesh->SetCollisionResponseToAllChannels(ECR_Block);
	HandMesh->SetGenerateOverlapEvents(false);
	HandMesh->SetNotifyRigidBodyCollision(true);

	TelegraphStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TelegraphStartPoint"));
	TelegraphStartPoint->SetupAttachment(HandMesh);

	HandMovementComponent = CreateDefaultSubobject<UPBGolemHandMovementComponent>(TEXT("HandMovementComponent"));
}

void APBGolemBossHand::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	static_cast<void>(DeltaSeconds);
	UpdateFistAimRotation();
}

void APBGolemBossHand::InitializeGolemHand(
	APBGolemBoss* NewOwnerBoss,
	EPBGolemBossHandType NewHandType,
	FVector NewDefaultOffset)
{
	OwnerBoss = NewOwnerBoss;
	HandType = NewHandType;

	if (HandMovementComponent)
	{
		HandMovementComponent->InitializeHandMovement(OwnerBoss, NewDefaultOffset);
	}
}

void APBGolemBossHand::MoveToOffset(FVector TargetOffset, float Duration)
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	if (HandMovementComponent)
	{
		HandMovementComponent->MoveToOffset(TargetOffset, Duration);
	}
}

void APBGolemBossHand::MoveToWorldLocation(FVector TargetWorldLocation, float Duration)
{
	MoveToWorldLocationInternal(TargetWorldLocation, Duration, false);
}

void APBGolemBossHand::MoveToWorldLocationInternal(FVector TargetWorldLocation, float Duration, bool IsIgnorePatternMovementLock)
{
	if (IsPatternMovementLocked && !IsIgnorePatternMovementLock)
	{
		return;
	}

	if (HandMovementComponent)
	{
		HandMovementComponent->MoveToWorldLocation(TargetWorldLocation, Duration);
	}
}

void APBGolemBossHand::MoveTelegraphStartToWorldLocation(FVector TargetWorldLocation, float Duration)
{
	const FVector ActorTargetLocation = CalculateActorTargetLocationForTelegraphStart(TargetWorldLocation);
	MoveToWorldLocation(ActorTargetLocation, Duration);
}

void APBGolemBossHand::MoveToWorldTransform(FTransform TargetTransform, float Duration)
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	SetActorRotation(TargetTransform.GetRotation());
	MoveToWorldLocation(TargetTransform.GetLocation(), Duration);
}

void APBGolemBossHand::MoveTelegraphStartToWorldLocationForPattern(FVector TargetWorldLocation, float Duration)
{
	const FVector ActorTargetLocation = CalculateActorTargetLocationForTelegraphStart(TargetWorldLocation);
	MoveToWorldLocationInternal(ActorTargetLocation, Duration, true);
}

void APBGolemBossHand::MoveToWorldTransformForPattern(FTransform TargetTransform, float Duration)
{
	const FVector TelegraphStartLocation = TelegraphStartPoint
		? TelegraphStartPoint->GetComponentLocation()
		: GetActorLocation();
	SetActorRotation(TargetTransform.GetRotation());
	if (TelegraphStartPoint)
	{
		AddActorWorldOffset(TelegraphStartLocation - TelegraphStartPoint->GetComponentLocation());
	}
	MoveToWorldLocationInternal(TargetTransform.GetLocation(), Duration, true);
}

void APBGolemBossHand::BeginPatternMovementLock()
{
	IsPatternMovementLocked = true;
	StopAutonomousMove();
}

void APBGolemBossHand::EndPatternMovementLock()
{
	IsPatternMovementLocked = false;
}

void APBGolemBossHand::ReturnToDefaultOffset(float Duration)
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	if (HandMovementComponent)
	{
		HandMovementComponent->ReturnToDefaultOffset(Duration);
	}
}

void APBGolemBossHand::LaunchFistAtActor(AActor* TargetActor, float Duration)
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	if (!TargetActor)
	{
		return;
	}

	LaunchFistAtLocation(TargetActor->GetActorLocation(), Duration);
}

void APBGolemBossHand::LaunchFistAtLocation(FVector TargetWorldLocation, float Duration)
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	StopAutonomousMove();
	BP_OnFistLaunchStarted(TargetWorldLocation);

	MoveTelegraphStartToWorldLocation(TargetWorldLocation, Duration);
}

void APBGolemBossHand::LaunchFistAtLocationForPattern(FVector TargetWorldLocation, FVector Direction, float Duration)
{
	FVector SafeDirection = Direction;
	SafeDirection.Z = 0.0f;
	SafeDirection = SafeDirection.GetSafeNormal();
	if (!SafeDirection.IsNearlyZero())
	{
		const FVector TelegraphStartLocation = TelegraphStartPoint
			? TelegraphStartPoint->GetComponentLocation()
			: GetActorLocation();
		SetActorRotation(SafeDirection.Rotation());
		if (TelegraphStartPoint)
		{
			AddActorWorldOffset(TelegraphStartLocation - TelegraphStartPoint->GetComponentLocation());
		}
	}

	BP_OnFistLaunchStarted(TargetWorldLocation);
	MoveTelegraphStartToWorldLocationForPattern(TargetWorldLocation, Duration);
}

void APBGolemBossHand::StartFistAimAtActor(AActor* TargetActor)
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemBossHand] StartFistAimAtActor failed. Hand=%s TargetActor=%s"),
			*GetNameSafe(this),
			*GetNameSafe(TargetActor));
		StopFistAim();
		return;
	}

	FistAimTargetActor = TargetActor;
	IsFistAiming = true;
	SetActorTickEnabled(true);
	UpdateFistAimRotation();

	UE_LOG(LogTemp, Log, TEXT("[GolemBossHand] StartFistAimAtActor. Hand=%s TargetActor=%s"),
		*GetNameSafe(this),
		*GetNameSafe(TargetActor));
}

void APBGolemBossHand::StopFistAim()
{
	if (IsFistAiming)
	{
		UE_LOG(LogTemp, Log, TEXT("[GolemBossHand] StopFistAim. Hand=%s TargetActor=%s"),
			*GetNameSafe(this),
			*GetNameSafe(FistAimTargetActor));
	}

	IsFistAiming = false;
	FistAimTargetActor = nullptr;
	SetActorTickEnabled(false);
}

void APBGolemBossHand::StartAutonomousMove()
{
	if (IsPatternMovementLocked)
	{
		return;
	}

	if (HandMovementComponent)
	{
		HandMovementComponent->StartAutonomousMove();
	}
}

void APBGolemBossHand::StopAutonomousMove()
{
	if (HandMovementComponent)
	{
		HandMovementComponent->StopAutonomousMove();
	}
}

void APBGolemBossHand::RequestIdleAnimationSync()
{
	BP_OnIdleAnimationSyncRequested(GetSyncedIdleAnimationPosition());
}

float APBGolemBossHand::GetSyncedIdleAnimationPosition() const
{
	return OwnerBoss ? OwnerBoss->GetSyncedGolemIdleAnimationPosition(IdleAnimationLength) : 0.0f;
}

EPBGolemBossHandType APBGolemBossHand::GetHandType() const
{
	return HandType;
}

UPBGolemHandMovementComponent* APBGolemBossHand::GetHandMovementComponent() const
{
	return HandMovementComponent;
}

USkeletalMeshComponent* APBGolemBossHand::GetHandMeshComponent() const
{
	return HandMesh;
}

USceneComponent* APBGolemBossHand::GetTelegraphStartComponent() const
{
	return IsValid(TelegraphStartPoint) ? TelegraphStartPoint.Get() : HandMesh.Get();
}

FVector APBGolemBossHand::CalculateActorTargetLocationForTelegraphStart(FVector TargetWorldLocation) const
{
	const USceneComponent* StartComponent = GetTelegraphStartComponent();
	if (!StartComponent)
	{
		return TargetWorldLocation;
	}

	const FVector CurrentActorLocation = GetActorLocation();
	const FVector CurrentStartLocation = StartComponent->GetComponentLocation();
	return CurrentActorLocation + (TargetWorldLocation - CurrentStartLocation);
}

void APBGolemBossHand::UpdateFistAimRotation()
{
	if (!IsFistAiming || !IsValid(FistAimTargetActor))
	{
		return;
	}

	const USceneComponent* StartComponent = GetTelegraphStartComponent();
	const FVector StartLocation = StartComponent ? StartComponent->GetComponentLocation() : GetActorLocation();
	FVector AimDirection = FistAimTargetActor->GetActorLocation() - StartLocation;
	AimDirection.Z = 0.0f;
	AimDirection = AimDirection.GetSafeNormal();

	if (AimDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemBossHand] UpdateFistAimRotation skipped. Hand=%s StartLocation=%s TargetActor=%s"),
			*GetNameSafe(this),
			*StartLocation.ToString(),
			*GetNameSafe(FistAimTargetActor));
		return;
	}

	SetActorRotation(AimDirection.Rotation());
}
