#include "PBGateAccelerationField.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldDebugDraw.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"

APBGateAccelerationField::APBGateAccelerationField()
{
#if ENABLE_DRAW_DEBUG
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif

	FieldArea = CreateDefaultSubobject<UBoxComponent>(TEXT("FieldArea"));
	FieldArea->SetupAttachment(SceneRoot);
	FieldArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FieldArea->SetGenerateOverlapEvents(false);

	RadialFieldArea = CreateDefaultSubobject<USphereComponent>(TEXT("RadialFieldArea"));
	RadialFieldArea->SetupAttachment(SceneRoot);
	RadialFieldArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RadialFieldArea->SetCollisionObjectType(ECC_WorldDynamic);
	RadialFieldArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	RadialFieldArea->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	RadialFieldArea->SetGenerateOverlapEvents(true);
	RadialFieldArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBGateAccelerationField::HandleFieldBeginOverlap);
	RadialFieldArea->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APBGateAccelerationField::HandleFieldEndOverlap);

	FieldVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldVisual"));
	FieldVisual->SetupAttachment(FieldArea);
	FieldVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FieldVisual->SetGenerateOverlapEvents(false);
	FieldVisual->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.05f));

	RefreshFieldGeometry();

	SetFieldActive(false);
}

void APBGateAccelerationField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshFieldGeometry();
}

void APBGateAccelerationField::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PBGateFieldDebugDraw::Draw(
		GetWorld(),
		GetActorLocation(),
		FieldRadius,
		FColor(64, 176, 255),
		TEXT("GATE ACCELERATION FIELD"),
		bHasDebugTriggerOrigin,
		DebugTriggerOrigin);
}

void APBGateAccelerationField::SetDebugTriggerOrigin(const FVector& InTriggerOrigin)
{
	bHasDebugTriggerOrigin = !InTriggerOrigin.ContainsNaN();
	DebugTriggerOrigin = bHasDebugTriggerOrigin ? InTriggerOrigin : FVector::ZeroVector;
}

void APBGateAccelerationField::StartActionForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	OverlappingActorCounts.Reset();
	AcceleratedActors.Reset();
	RefreshFieldGeometry();
	SetFieldActive(true);
	Super::StartActionForActor(Bumper, InteractionActor);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActiveDurationTimerHandle,
			this,
			&APBGateAccelerationField::HandleActiveDurationFinished,
			ActiveDuration,
			false);
	}
}

void APBGateAccelerationField::DeactivateSummon()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	OverlappingActorCounts.Reset();
	AcceleratedActors.Reset();
	SetFieldActive(false);
	Super::DeactivateSummon();
}

void APBGateAccelerationField::ConfigureField(
	const float InSpeedBoostPercent,
	const float InActiveDuration)
{
	SpeedBoostPercent = FMath::IsFinite(InSpeedBoostPercent)
		? FMath::Clamp(InSpeedBoostPercent, 0.0f, 500.0f)
		: 0.0f;
	ActiveDuration = FMath::IsFinite(InActiveDuration)
		? FMath::Clamp(InActiveDuration, 0.1f, 60.0f)
		: 5.0f;
}

void APBGateAccelerationField::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	OverlappingActorCounts.Reset();
	AcceleratedActors.Reset();
	Super::EndPlay(EndPlayReason);
}

void APBGateAccelerationField::SetFieldActive(const bool bIsActive)
{
	FieldArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RadialFieldArea->SetCollisionEnabled(
		bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	FieldVisual->SetVisibility(bIsActive, true);
#if ENABLE_DRAW_DEBUG
	SetActorTickEnabled(bIsActive);
#endif
}

void APBGateAccelerationField::RefreshFieldGeometry()
{
	FieldRadius = FMath::Clamp(
		FieldRadius,
		PBGateFieldTuning::MinimumRadius,
		PBGateFieldTuning::MaximumRadius);
	FieldArea->SetBoxExtent(FVector(
		FieldRadius,
		FieldRadius,
		PBGateFieldTuning::CollisionHalfHeight));
	RadialFieldArea->SetSphereRadius(FieldRadius, true);

	FVector VisualScale = FieldVisual->GetRelativeScale3D();
	const float DiameterScale = FieldRadius / PBGateFieldTuning::BasicShapeRadiusAtScaleOne;
	VisualScale.X = DiameterScale;
	VisualScale.Y = DiameterScale;
	FieldVisual->SetRelativeScale3D(VisualScale);
}

bool APBGateAccelerationField::ApplyAcceleration(AActor* InteractionActor) const
{
	if (!IsValid(InteractionActor) || SpeedBoostPercent <= 0.0f)
	{
		return false;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(InteractionActor);
	if (!Movable)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration skipped because the target has no movement interface. Target=%s"),
			*GetNameSafe(InteractionActor));
		return false;
	}

	FVector CurrentVelocity = Movable->GetVelocity();
	CurrentVelocity.Z = 0.0f;
	if (!FMath::IsFinite(CurrentVelocity.X)
		|| !FMath::IsFinite(CurrentVelocity.Y)
		|| CurrentVelocity.IsNearlyZero())
	{
		return false;
	}

	const FVector AddedVelocity = CurrentVelocity * (SpeedBoostPercent / 100.0f);
	const float BeforeSpeed = CurrentVelocity.Size2D();
	Movable->AddVelocity(AddedVelocity);
	const float AfterSpeed = Movable->GetVelocity().Size2D();

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Gate acceleration applied. Field=%s Target=%s Percent=%.1f BeforeSpeed=%.1f AfterSpeed=%.1f"),
		*GetNameSafe(this),
		*GetNameSafe(InteractionActor),
		SpeedBoostPercent,
		BeforeSpeed,
		AfterSpeed);
	return AfterSpeed > BeforeSpeed + KINDA_SMALL_NUMBER;
}

void APBGateAccelerationField::HandleActiveDurationFinished()
{
	FinishAction();
}

void APBGateAccelerationField::HandleFieldBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor)
		|| !PBInterfaceUtils::FindInterface<IMovable>(OtherActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> ActorKey = OtherActor;
	int32& OverlapCount = OverlappingActorCounts.FindOrAdd(ActorKey);
	if (OverlapCount == 0 && !AcceleratedActors.Contains(ActorKey))
	{
		if (ApplyAcceleration(OtherActor))
		{
			AcceleratedActors.Add(ActorKey);
		}
	}
	++OverlapCount;
}

void APBGateAccelerationField::HandleFieldEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> ActorKey = OtherActor;
	int32* OverlapCount = OverlappingActorCounts.Find(ActorKey);
	if (OverlapCount == nullptr)
	{
		return;
	}

	--(*OverlapCount);
	if (*OverlapCount <= 0)
	{
		OverlappingActorCounts.Remove(ActorKey);
	}
}
