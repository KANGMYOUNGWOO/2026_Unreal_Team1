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
	CurrentDirection = NormalizeDirection2D(GetActorForwardVector());

	UpdateTrackedPinballTransform();

	UE_LOG(LogTemp, Log, TEXT("[ChargeTelegraph] InitTelegraph. Telegraph=%s StartLocation=%s TargetLocation=%s Direction=%s Length=%.2f StartActor=%s"),
		*GetNameSafe(this),
		*ChargeStartLocation.ToString(),
		*CurrentTargetLocation.ToString(),
		*CurrentDirection.ToString(),
		CurrentLength,
		*GetNameSafe(ChargeStartActor));

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

	UE_LOG(LogTemp, Log, TEXT("[ChargeTelegraph] InitChargeTelegraph. Telegraph=%s StartLocation=%s Direction=%s Length=%.2f Scale=%s"),
		*GetNameSafe(this),
		*StartLocation.ToString(),
		*Direction.ToString(),
		Length,
		*InScale.ToString());
}

void APBBossChargeTelegraph::UpdateChargeTelegraphTransform(
	const FVector& StartLocation,
	const FVector& Direction,
	float Length)
{
	const FVector SafeDirection = NormalizeDirection2D(Direction);
	const float SafeLength = FMath::Max(0.0f, Length);

	SetActorLocation(StartLocation);
	SetActorRotation(SafeDirection.Rotation());
	UpdateVisualComponentRotations();

	ChargeStartLocation = StartLocation;
	CurrentDirection = SafeDirection;
	CurrentLength = SafeLength;
	CurrentTargetLocation = StartLocation + SafeDirection * SafeLength;
}

void APBBossChargeTelegraph::SetChargeStartActor(AActor* NewChargeStartActor)
{
	ChargeStartActor = NewChargeStartActor;
	ChargeStartComponent = nullptr;
	UpdateTrackedPinballTransform();

	UE_LOG(LogTemp, Log, TEXT("[ChargeTelegraph] SetChargeStartActor. Telegraph=%s StartActor=%s ActorLocation=%s StartLocation=%s TargetLocation=%s Direction=%s Length=%.2f"),
		*GetNameSafe(this),
		*GetNameSafe(ChargeStartActor),
		ChargeStartActor ? *ChargeStartActor->GetActorLocation().ToString() : TEXT("None"),
		*ChargeStartLocation.ToString(),
		*CurrentTargetLocation.ToString(),
		*CurrentDirection.ToString(),
		CurrentLength);
}

void APBBossChargeTelegraph::SetChargeStartComponent(USceneComponent* NewChargeStartComponent)
{
	ChargeStartComponent = NewChargeStartComponent;
	ChargeStartActor = NewChargeStartComponent ? NewChargeStartComponent->GetOwner() : nullptr;
	UpdateTrackedPinballTransform();

	UE_LOG(LogTemp, Log, TEXT("[ChargeTelegraph] SetChargeStartComponent. Telegraph=%s StartComponent=%s ComponentLocation=%s Owner=%s StartLocation=%s TargetLocation=%s Direction=%s Length=%.2f"),
		*GetNameSafe(this),
		*GetNameSafe(ChargeStartComponent),
		ChargeStartComponent ? *ChargeStartComponent->GetComponentLocation().ToString() : TEXT("None"),
		*GetNameSafe(ChargeStartActor),
		*ChargeStartLocation.ToString(),
		*CurrentTargetLocation.ToString(),
		*CurrentDirection.ToString(),
		CurrentLength);
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
	UE_LOG(LogTemp, Log, TEXT("[ChargeTelegraph] Finished. Telegraph=%s StartActor=%s StartLocation=%s TargetLocation=%s Direction=%s Length=%.2f"),
		*GetNameSafe(this),
		*GetNameSafe(ChargeStartActor),
		*ChargeStartLocation.ToString(),
		*CurrentTargetLocation.ToString(),
		*CurrentDirection.ToString(),
		CurrentLength);
	OnChargeTelegraphFinished.Broadcast(CurrentTargetLocation, CurrentDirection);
	DestroyTelegraph();
}

void APBBossChargeTelegraph::UpdateTrackedPinballTransform()
{
	UpdateChargeStartLocation();

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

void APBBossChargeTelegraph::UpdateChargeStartLocation()
{
	if (IsValid(ChargeStartComponent))
	{
		ChargeStartLocation = ChargeStartComponent->GetComponentLocation();
		return;
	}

	if (IsValid(ChargeStartActor))
	{
		ChargeStartLocation = ChargeStartActor->GetActorLocation();
	}
}

void APBBossChargeTelegraph::UpdateVisualComponentRotations()
{
	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (!SceneComponent || SceneComponent == GetRootComponent())
		{
			continue;
		}

		const TObjectKey<USceneComponent> SceneComponentKey(SceneComponent);
		const FRotator InitialRelativeRotation = InitialRelativeRotationMap.FindOrAdd(
			SceneComponentKey,
			SceneComponent->GetRelativeRotation());

		SceneComponent->SetRelativeRotation(InitialRelativeRotation + VisualRotationOffset);
	}
}

AActor* APBBossChargeTelegraph::FindPinballActor() const
{
	const UWorld* World = GetWorld();
	return World ? UGameplayStatics::GetActorOfClass(World, APBBallBase::StaticClass()) : nullptr;
}

FVector APBBossChargeTelegraph::CalculateDirectionToTarget(const FVector& TargetLocation) const
{
	return NormalizeDirection2D(TargetLocation - ChargeStartLocation);
}

FVector APBBossChargeTelegraph::NormalizeDirection2D(const FVector& Direction) const
{
	FVector NormalizedDirection = Direction;
	NormalizedDirection.Z = 0.0f;
	NormalizedDirection = NormalizedDirection.GetSafeNormal();
	return NormalizedDirection.IsNearlyZero() ? FVector::ForwardVector : NormalizedDirection;
}
