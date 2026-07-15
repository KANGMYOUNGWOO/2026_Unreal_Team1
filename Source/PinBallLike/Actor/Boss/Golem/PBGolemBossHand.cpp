#include "PBGolemBossHand.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossHitEffectComponent.h"
#include "PBGolemBoss.h"
#include "PBGolemHandMovementComponent.h"
#include "TimerManager.h"

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
	HandMesh->SetGenerateOverlapEvents(true);
	HandMesh->SetNotifyRigidBodyCollision(true);

	TelegraphStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TelegraphStartPoint"));
	TelegraphStartPoint->SetupAttachment(HandMesh);

	HandMovementComponent = CreateDefaultSubobject<UPBGolemHandMovementComponent>(TEXT("HandMovementComponent"));
	HitEffectComponent = CreateDefaultSubobject<UPBBossHitEffectComponent>(TEXT("HitEffectComponent"));
}

void APBGolemBossHand::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	static_cast<void>(DeltaSeconds);
	UpdateFistAimRotation();
}

void APBGolemBossHand::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RegenerationTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APBGolemBossHand::DamageToBoss_Implementation(int32 DamageAmount)
{
	ApplyHandDamage(DamageAmount);
}

void APBGolemBossHand::InitializeGolemHand(
	APBGolemBoss* NewOwnerBoss,
	EPBGolemBossHandType NewHandType,
	FVector NewDefaultOffset)
{
	OwnerBoss = NewOwnerBoss;
	HandType = NewHandType;
	MaxHandHP = FMath::Max(1, MaxHandHP);
	CurrentHandHP = MaxHandHP;
	IsHandDestroyedValue = false;

	if (HandMovementComponent)
	{
		HandMovementComponent->InitializeHandMovement(OwnerBoss, NewDefaultOffset);
	}

	OnHandHPChanged.Broadcast(CurrentHandHP, MaxHandHP);
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
	static_cast<void>(Direction);

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
	if (IsPatternMovementLocked || !IsHandAvailable())
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

void APBGolemBossHand::SetIsPunching(bool NewIsPunching)
{
	IsPunching = NewIsPunching;
}

float APBGolemBossHand::GetPunchAlpha() const
{
	return IsPunching ? 1.0f : 0.0f;
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

void APBGolemBossHand::ApplyHandDamage(int32 DamageAmount)
{
	if (!IsHandAvailable() || IsPatternMovementLocked || DamageAmount <= 0)
	{
		return;
	}

	CurrentHandHP = FMath::Max(0, CurrentHandHP - DamageAmount);
	if (HitEffectComponent)
	{
		HitEffectComponent->PlayHitEffect();
	}
	OnHandHPChanged.Broadcast(CurrentHandHP, MaxHandHP);

	if (CurrentHandHP <= 0)
	{
		DestroyHand();
	}
}

bool APBGolemBossHand::IsHandAvailable() const
{
	return !IsHandDestroyedValue;
}

int32 APBGolemBossHand::GetCurrentHandHP() const
{
	return CurrentHandHP;
}

int32 APBGolemBossHand::GetMaxHandHP() const
{
	return MaxHandHP;
}

void APBGolemBossHand::DestroyHand()
{
	if (IsHandDestroyedValue)
	{
		return;
	}

	IsHandDestroyedValue = true;
	StopFistAim();
	IsPatternMovementLocked = false;
	if (HandMovementComponent)
	{
		HandMovementComponent->StopMove();
	}

	SetHandActive(false);
	BP_OnHandDestroyed();
	OnHandDestroyed.Broadcast();

	if (OwnerBoss)
	{
		OwnerBoss->HandleGolemHandDestroyed(this, DestroyedGroggyAmount);
	}

	if (HandMovementComponent)
	{
		HandMovementComponent->StopMove();
	}

	if (RegenerationDelay <= 0.0f)
	{
		RegenerateHand();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RegenerationTimerHandle,
		this,
		&APBGolemBossHand::RegenerateHand,
		RegenerationDelay,
		false);
}

void APBGolemBossHand::RegenerateHand()
{
	GetWorldTimerManager().ClearTimer(RegenerationTimerHandle);
	CurrentHandHP = MaxHandHP;
	IsHandDestroyedValue = false;
	IsPatternMovementLocked = false;

	if (HandMovementComponent)
	{
		HandMovementComponent->InitializeHandMovement(
			OwnerBoss,
			HandMovementComponent->GetDefaultOffset());
	}

	SetHandActive(true);
	OnHandHPChanged.Broadcast(CurrentHandHP, MaxHandHP);
	BP_OnHandRegenerated();
	OnHandRegenerated.Broadcast();

	if (OwnerBoss && OwnerBoss->IsIdleState())
	{
		StartAutonomousMove();
	}
}

void APBGolemBossHand::SetHandActive(bool IsActive)
{
	if (!HandMesh)
	{
		return;
	}

	HandMesh->SetVisibility(IsActive, true);
	HandMesh->SetCollisionEnabled(IsActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
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
