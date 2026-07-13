// Fill out your copyright notice in the Description page of Project Settings.

#include "PBGateAccelerationField.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"

APBGateAccelerationField::APBGateAccelerationField()
{
	PrimaryActorTick.bCanEverTick = false;

	FieldArea = CreateDefaultSubobject<UBoxComponent>(TEXT("FieldArea"));
	FieldArea->SetupAttachment(SceneRoot);
	FieldArea->SetBoxExtent(FVector(160.0f, 70.0f, 80.0f));
	FieldArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FieldArea->SetCollisionObjectType(ECC_WorldDynamic);
	FieldArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	FieldArea->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	FieldArea->SetGenerateOverlapEvents(true);
	FieldArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBGateAccelerationField::HandleFieldBeginOverlap);
	FieldArea->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APBGateAccelerationField::HandleFieldEndOverlap);

	FieldVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldVisual"));
	FieldVisual->SetupAttachment(FieldArea);
	FieldVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FieldVisual->SetGenerateOverlapEvents(false);

	SetFieldActive(false);
}

void APBGateAccelerationField::StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	OverlappingBallCounts.Reset();
	SetFieldActive(true);
	Super::StartAction(Bumper, Ball);

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

	OverlappingBallCounts.Reset();
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

	OverlappingBallCounts.Reset();
	Super::EndPlay(EndPlayReason);
}

void APBGateAccelerationField::SetFieldActive(const bool bIsActive)
{
	FieldArea->SetCollisionEnabled(
		bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	FieldVisual->SetVisibility(bIsActive, true);
}

void APBGateAccelerationField::ApplyAcceleration(APBBallBase* Ball) const
{
	if (!IsValid(Ball) || SpeedBoostPercent <= 0.0f)
	{
		return;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball);
	if (!Movable)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration skipped because Ball has no movement interface. Ball=%s"),
			*GetNameSafe(Ball));
		return;
	}

	FVector CurrentVelocity = Movable->GetVelocity();
	CurrentVelocity.Z = 0.0f;
	if (!FMath::IsFinite(CurrentVelocity.X)
		|| !FMath::IsFinite(CurrentVelocity.Y)
		|| CurrentVelocity.IsNearlyZero())
	{
		return;
	}

	const FVector AddedVelocity = CurrentVelocity * (SpeedBoostPercent / 100.0f);
	const float BeforeSpeed = CurrentVelocity.Size2D();
	Movable->AddVelocity(AddedVelocity);
	const float AfterSpeed = Movable->GetVelocity().Size2D();

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Gate acceleration applied. Field=%s Ball=%s Percent=%.1f BeforeSpeed=%.1f AfterSpeed=%.1f"),
		*GetNameSafe(this),
		*GetNameSafe(Ball),
		SpeedBoostPercent,
		BeforeSpeed,
		AfterSpeed);
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
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	int32& OverlapCount = OverlappingBallCounts.FindOrAdd(BallKey);
	if (OverlapCount == 0)
	{
		ApplyAcceleration(Ball);
	}
	++OverlapCount;
}

void APBGateAccelerationField::HandleFieldEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		return;
	}

	const TWeakObjectPtr<APBBallBase> BallKey = Ball;
	int32* OverlapCount = OverlappingBallCounts.Find(BallKey);
	if (OverlapCount == nullptr)
	{
		return;
	}

	--(*OverlapCount);
	if (*OverlapCount <= 0)
	{
		OverlappingBallCounts.Remove(BallKey);
	}
}
