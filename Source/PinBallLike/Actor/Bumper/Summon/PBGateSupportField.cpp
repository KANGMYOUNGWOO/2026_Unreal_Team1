#include "PBGateSupportField.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PinBallLike/Actor/Bumper/Reward/PBBumperRewardUtils.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldDebugDraw.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APBGateSupportField::APBGateSupportField()
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
		&APBGateSupportField::HandleFieldBeginOverlap);

	FieldVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldVisual"));
	FieldVisual->SetupAttachment(FieldArea);
	FieldVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FieldVisual->SetGenerateOverlapEvents(false);
	FieldVisual->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.05f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		FieldVisual->SetStaticMesh(CubeMesh.Object);
	}

	FieldLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FieldLight"));
	FieldLight->SetupAttachment(FieldArea);
	FieldLight->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	FieldLight->SetIntensity(1200.0f);
	FieldLight->SetCastShadows(false);

	RefreshFieldGeometry();

	SetFieldActive(false);
}

void APBGateSupportField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshFieldGeometry();
}

void APBGateSupportField::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PBGateFieldDebugDraw::Draw(
		GetWorld(),
		GetActorLocation(),
		FieldRadius,
		DebugFieldColor,
		TEXT("GATE SUPPORT FIELD"),
		bHasDebugTriggerOrigin,
		DebugTriggerOrigin);
}

void APBGateSupportField::SetDebugTriggerOrigin(const FVector& InTriggerOrigin)
{
	bHasDebugTriggerOrigin = !InTriggerOrigin.ContainsNaN();
	DebugTriggerOrigin = bHasDebugTriggerOrigin ? InTriggerOrigin : FVector::ZeroVector;
}

void APBGateSupportField::StartActionForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	RewardedActors.Reset();
	RefreshFieldGeometry();
	SetFieldActive(true);
	Super::StartActionForActor(Bumper, InteractionActor);

	TryApplyReward(InteractionActor);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActiveDurationTimerHandle,
			this,
			&APBGateSupportField::HandleActiveDurationFinished,
			ActiveDuration,
			false);
	}
}

void APBGateSupportField::DeactivateSummon()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	RewardedActors.Reset();
	SetFieldActive(false);
	Super::DeactivateSummon();
}

void APBGateSupportField::ConfigureField(
	const EPBBumperRewardType InRewardType,
	const FName InResourceName,
	const FName InStatusEffectId,
	const FName InTimedEffectSourceId,
	const FName InTimedStatName,
	const float InRewardPower,
	const int32 InTriggerCount,
	const float InActiveDuration,
	const FLinearColor& InFieldColor)
{
	RewardType = InRewardType;
	ResourceName = InResourceName;
	StatusEffectId = InStatusEffectId;
	TimedEffectSourceId = InTimedEffectSourceId;
	TimedStatName = InTimedStatName;
	RewardPower = FMath::IsFinite(InRewardPower) ? FMath::Max(InRewardPower, 0.0f) : 0.0f;
	TriggerCount = FMath::Max(InTriggerCount, 0);
	ActiveDuration = FMath::IsFinite(InActiveDuration)
		? FMath::Clamp(InActiveDuration, 0.5f, 60.0f)
		: 5.0f;
	FieldLight->SetLightColor(InFieldColor);
	DebugFieldColor = InFieldColor.ToFColor(true);
	RefreshFieldGeometry();
}

void APBGateSupportField::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}

	RewardedActors.Reset();
	Super::EndPlay(EndPlayReason);
}

void APBGateSupportField::SetFieldActive(const bool bNewActive)
{
	FieldArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RadialFieldArea->SetCollisionEnabled(
		bNewActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	FieldVisual->SetVisibility(bNewActive, true);
	FieldLight->SetVisibility(bNewActive, true);
#if ENABLE_DRAW_DEBUG
	SetActorTickEnabled(bNewActive);
#endif
}

void APBGateSupportField::RefreshFieldGeometry()
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
	FieldLight->SetAttenuationRadius(FieldRadius * 2.0f);
}

bool APBGateSupportField::TryApplyReward(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || RewardedActors.Contains(TargetActor))
	{
		return false;
	}

	const FPBBumperRewardApplyResult Result = PBBumperRewardUtils::ApplyReward(
		TargetActor,
		RewardType,
		ResourceName,
		StatusEffectId,
		RewardPower,
		TimedEffectSourceId,
		ActiveDuration,
		TimedStatName,
		TriggerCount);
	if (!Result.bApplied)
	{
		return false;
	}

	RewardedActors.Add(TargetActor);
	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Support field reward applied. Field=%s Target=%s RewardType=%s AppliedValue=%.2f AppliedCount=%d"),
		*GetNameSafe(this),
		*GetNameSafe(TargetActor),
		*UEnum::GetValueAsString(RewardType),
		Result.AppliedValue,
		Result.AppliedCount);
	return true;
}

void APBGateSupportField::HandleActiveDurationFinished()
{
	FinishAction();
}

void APBGateSupportField::HandleFieldBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComp);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(bFromSweep);
	static_cast<void>(SweepResult);
	TryApplyReward(OtherActor);
}
