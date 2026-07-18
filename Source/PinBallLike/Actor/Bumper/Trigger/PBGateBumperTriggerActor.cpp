// Fill out your copyright notice in the Description page of Project Settings.


#include "PBGateBumperTriggerActor.h"

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBGateBumperTriggerActor::APBGateBumperTriggerActor()
{
	TriggerType = EPBBumperTriggerType::PassCount;
}

void APBGateBumperTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	RegisterGateAreas();
	InitializeFlagSpinReaction();
	InitializeGaugeVisual();
	OnTriggerProgressChanged.AddUniqueDynamic(
		this,
		&APBGateBumperTriggerActor::HandleTriggerProgressChanged);
	HandleTriggerProgressChanged(GetCurrentTriggerCount(), GetRequiredTriggerCount());
}

void APBGateBumperTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnTriggerProgressChanged.RemoveDynamic(
		this,
		&APBGateBumperTriggerActor::HandleTriggerProgressChanged);

	for (UPrimitiveComponent* GateArea : GateAreas)
	{
		if (!IsValid(GateArea))
		{
			continue;
		}

		GateArea->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&APBGateBumperTriggerActor::HandleGateBeginOverlap);
		GateArea->OnComponentEndOverlap.RemoveDynamic(
			this,
			&APBGateBumperTriggerActor::HandleGateEndOverlap);
	}

	GateAreas.Reset();
	PassingActorOverlapCounts.Reset();
	FlagSpinActorOverlapCounts.Reset();
	GaugeMaterial = nullptr;
	FlagVisualMesh = nullptr;
	LastFlagSpinAngularVelocity = FVector::ZeroVector;
	bIsFlagSpinReactionReady = false;

	Super::EndPlay(EndPlayReason);
}

void APBGateBumperTriggerActor::RegisterGateAreas()
{
	// BP에서 만든 통과 판정 영역을 태그 기준으로 찾아 overlap 이벤트에 연결한다.
	TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		GateAreaTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		SetupGateArea(Cast<UPrimitiveComponent>(TaggedComponent));
	}
}

void APBGateBumperTriggerActor::SetupGateArea(UPrimitiveComponent* GateArea)
{
	if (!IsValid(GateArea) || GateAreas.Contains(GateArea))
	{
		return;
	}

	// 같은 컴포넌트가 중복 등록되어도 overlap이 한 번만 오도록 바인딩한다.
	GateArea->SetGenerateOverlapEvents(true);
	GateArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBGateBumperTriggerActor::HandleGateBeginOverlap);
	GateArea->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APBGateBumperTriggerActor::HandleGateEndOverlap);
	GateAreas.Add(GateArea);
}

void APBGateBumperTriggerActor::InitializeGaugeVisual()
{
	auto TryCreateGaugeMaterial = [this](UMeshComponent* VisualMesh) -> bool
	{
		if (!IsValid(VisualMesh) || !VisualMesh->GetMaterial(GaugeMaterialIndex))
		{
			return false;
		}

		GaugeMaterial = VisualMesh->CreateDynamicMaterialInstance(GaugeMaterialIndex);
		return IsValid(GaugeMaterial);
	};

	const TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UMeshComponent::StaticClass(),
		GaugeVisualTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		if (TryCreateGaugeMaterial(Cast<UMeshComponent>(TaggedComponent)))
		{
			return;
		}
	}

	if (TryCreateGaugeMaterial(FlagVisualMesh))
	{
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[Bumper] Gate gauge visual is missing. Trigger=%s Tag=%s MaterialIndex=%d"),
		*GetNameSafe(this),
		*GaugeVisualTag.ToString(),
		GaugeMaterialIndex);
}

void APBGateBumperTriggerActor::InitializeFlagSpinReaction()
{
	bIsFlagSpinReactionReady = false;
	LastFlagSpinAngularVelocity = FVector::ZeroVector;
	if (!bEnableFlagSpinReaction)
	{
		return;
	}

	FlagVisualMesh = FindFlagVisualMesh();
	if (!IsValid(FlagVisualMesh))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate Flag visual mesh is missing. Trigger=%s Tag=%s Bone=%s"),
			*GetNameSafe(this),
			*FlagVisualTag.ToString(),
			*FlagBoneName.ToString());
		return;
	}

	if (!FlagVisualMesh->GetPhysicsAsset())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate Flag physics asset is missing. Trigger=%s Mesh=%s"),
			*GetNameSafe(this),
			*GetNameSafe(FlagVisualMesh));
		FlagVisualMesh = nullptr;
		return;
	}

	const ECollisionEnabled::Type PreviousCollisionEnabled =
		FlagVisualMesh->GetCollisionEnabled();
	const FCollisionResponseContainer PreviousCollisionResponses =
		FlagVisualMesh->GetCollisionResponseToChannels();
	FlagVisualMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	FlagVisualMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlagVisualMesh->SetEnablePhysicsBlending(true);
	FlagVisualMesh->SetBodySimulatePhysics(FlagBoneName, true);
	FlagVisualMesh->SetAllBodiesBelowPhysicsBlendWeight(FlagBoneName, 1.0f, false, true);
	if (!FlagVisualMesh->IsSimulatingPhysics(FlagBoneName))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate Flag body could not start simulation. Trigger=%s Bone=%s"),
			*GetNameSafe(this),
			*FlagBoneName.ToString());
		FlagVisualMesh->SetCollisionEnabled(PreviousCollisionEnabled);
		FlagVisualMesh->SetCollisionResponseToChannels(PreviousCollisionResponses);
		FlagVisualMesh = nullptr;
		return;
	}

	const FName ConstraintName = FlagConstraintName.IsNone()
		? FlagBoneName
		: FlagConstraintName;
	FConstraintInstance* FlagConstraint = FlagVisualMesh->FindConstraintInstance(ConstraintName);
	if (!FlagConstraint)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate Flag constraint is missing. Trigger=%s Constraint=%s"),
			*GetNameSafe(this),
			*ConstraintName.ToString());
		FlagVisualMesh->SetBodySimulatePhysics(FlagBoneName, false);
		FlagVisualMesh->SetCollisionEnabled(PreviousCollisionEnabled);
		FlagVisualMesh->SetCollisionResponseToChannels(PreviousCollisionResponses);
		FlagVisualMesh = nullptr;
		return;
	}

	const FVector VisualSpinAxis = CalculateFlagSpinAxis();
	const int32 FlagBoneIndex = FlagVisualMesh->GetBoneIndex(FlagBoneName);
	if (VisualSpinAxis.IsNearlyZero() || FlagBoneIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate Flag spin axis could not be resolved. Trigger=%s Flag=%s End=%s"),
			*GetNameSafe(this),
			*FlagBoneName.ToString(),
			*FlagEndBoneName.ToString());
		FlagVisualMesh->SetBodySimulatePhysics(FlagBoneName, false);
		FlagVisualMesh->SetCollisionEnabled(PreviousCollisionEnabled);
		FlagVisualMesh->SetCollisionResponseToChannels(PreviousCollisionResponses);
		FlagVisualMesh = nullptr;
		return;
	}

	const FVector ConstraintTwistAxis = FlagVisualMesh->GetBoneTransform(FlagBoneIndex)
		.TransformVectorNoScale(FlagConstraint->PriAxis1)
		.GetSafeNormal();
	const float TwistAxisAlignment = FMath::Abs(FVector::DotProduct(
		VisualSpinAxis,
		ConstraintTwistAxis));
	if (TwistAxisAlignment < 0.95f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate Flag visual axis and constraint Twist axis are misaligned. Trigger=%s Alignment=%.3f Visual=%s Constraint=%s"),
			*GetNameSafe(this),
			TwistAxisAlignment,
			*VisualSpinAxis.ToCompactString(),
			*ConstraintTwistAxis.ToCompactString());
	}

	FlagConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	FlagConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	FlagConstraint->SetAngularTwistLimit(
		EAngularConstraintMotion::ACM_Free,
		0.0f);

	FlagConstraint->SetDriveParams(
		FVector::ZeroVector,
		FVector::ZeroVector,
		FVector::ZeroVector,
		FVector::ZeroVector,
		FVector(0.0f, FMath::Max(FlagSpinDamping, 0.0f), 0.0f),
		FVector::ZeroVector,
		EAngularDriveMode::TwistAndSwing);
	FlagConstraint->SetAngularVelocityTarget(FVector::ZeroVector);
	FlagConstraint->SetAngularDriveAccelerationMode(true);
	FlagVisualMesh->SetPhysicsMaxAngularVelocityInRadians(
		FMath::Max3(MaximumFlagSpinAngularSpeed, MinimumFlagSpinAngularSpeed, 0.0f),
		false,
		FlagBoneName);

	FlagVisualMesh->WakeAllRigidBodies();
	bIsFlagSpinReactionReady = true;
	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Gate Flag spin is ready. Trigger=%s Mesh=%s Bone=%s SpinAxis=%s TwistAlignment=%.3f"),
		*GetNameSafe(this),
		*GetNameSafe(FlagVisualMesh),
		*FlagBoneName.ToString(),
		*VisualSpinAxis.ToCompactString(),
		TwistAxisAlignment);
}

USkeletalMeshComponent* APBGateBumperTriggerActor::FindFlagVisualMesh() const
{
	TInlineComponentArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetComponents(SkeletalMeshes);

	for (const bool bRequireTag : {true, false})
	{
		for (USkeletalMeshComponent* SkeletalMesh : SkeletalMeshes)
		{
			if (!IsValid(SkeletalMesh)
				|| SkeletalMesh->GetBoneIndex(FlagBoneName) == INDEX_NONE
				|| (bRequireTag && !FlagVisualTag.IsNone()
					&& !SkeletalMesh->ComponentHasTag(FlagVisualTag)))
			{
				continue;
			}

			return SkeletalMesh;
		}

		if (FlagVisualTag.IsNone())
		{
			break;
		}
	}

	return nullptr;
}

FVector APBGateBumperTriggerActor::CalculateFlagSpinAxis() const
{
	if (!IsValid(FlagVisualMesh)
		|| FlagVisualMesh->GetBoneIndex(FlagBoneName) == INDEX_NONE
		|| FlagVisualMesh->GetBoneIndex(FlagEndBoneName) == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}

	const FVector FlagPivot = FlagVisualMesh->GetBoneLocation(
		FlagBoneName,
		EBoneSpaces::WorldSpace);
	const FVector FlagEnd = FlagVisualMesh->GetBoneLocation(
		FlagEndBoneName,
		EBoneSpaces::WorldSpace);
	return (FlagEnd - FlagPivot).GetSafeNormal();
}

bool APBGateBumperTriggerActor::RegisterFlagSpinActorOverlap(AActor* InteractionActor)
{
	if (!IsValid(InteractionActor))
	{
		return false;
	}

	int32& OverlapCount = FlagSpinActorOverlapCounts.FindOrAdd(InteractionActor);
	++OverlapCount;
	return OverlapCount == 1;
}

void APBGateBumperTriggerActor::UnregisterFlagSpinActorOverlap(AActor* InteractionActor)
{
	if (!IsValid(InteractionActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> ActorKey = InteractionActor;
	int32* OverlapCount = FlagSpinActorOverlapCounts.Find(ActorKey);
	if (!OverlapCount)
	{
		return;
	}

	--(*OverlapCount);
	if (*OverlapCount <= 0)
	{
		FlagSpinActorOverlapCounts.Remove(ActorKey);
	}
}

void APBGateBumperTriggerActor::ApplyFlagSpinReaction(
	AActor* InteractionActor,
	const UPrimitiveComponent* GateArea)
{
	if (!bIsFlagSpinReactionReady
		|| !IsValid(FlagVisualMesh)
		|| !IsValid(InteractionActor))
	{
		return;
	}

	const IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(InteractionActor);
	if (!Movable)
	{
		return;
	}

	const FVector BallVelocity = Movable->GetVelocity();
	const FVector SpinAxis = CalculateFlagSpinAxis();
	if (BallVelocity.ContainsNaN() || SpinAxis.IsNearlyZero())
	{
		return;
	}

	const FVector FlagPivot = FlagVisualMesh->GetBoneLocation(
		FlagBoneName,
		EBoneSpaces::WorldSpace);
	const FVector PlanarVelocity = FVector::VectorPlaneProject(BallVelocity, SpinAxis);
	const float BallSpeed = PlanarVelocity.Size();
	if (!FMath::IsFinite(BallSpeed)
		|| BallSpeed < FMath::Max(MinimumFlagSpinBallSpeed, 0.0f))
	{
		return;
	}

	const FVector BallDirection = PlanarVelocity / BallSpeed;
	const FVector PlanarLever = FVector::VectorPlaneProject(
		InteractionActor->GetActorLocation() - FlagPivot,
		SpinAxis);
	const FVector SpinSideAxis = FVector::CrossProduct(BallDirection, SpinAxis).GetSafeNormal();
	if (SpinSideAxis.IsNearlyZero())
	{
		return;
	}

	const float SignedSpinOffset = FVector::DotProduct(PlanarLever, SpinSideAxis);
	float GateHalfWidth = PlanarLever.Size();
	if (IsValid(GateArea))
	{
		const FVector BoundsExtent = GateArea->Bounds.BoxExtent;
		const FVector AbsoluteSideAxis = SpinSideAxis.GetAbs();
		GateHalfWidth = FVector::DotProduct(BoundsExtent, AbsoluteSideAxis);
	}
	const float FullSpinDistance = FMath::Max(
		GateHalfWidth * FMath::Max(FlagFullSpinOffsetRatio, 0.01f),
		1.0f);
	const float RawSpinWeight = FMath::Clamp(
		FMath::Abs(SignedSpinOffset) / FullSpinDistance,
		0.0f,
		1.0f);
	const float SpinWeight = FMath::SmoothStep(0.0f, 1.0f, RawSpinWeight);
	if (SpinWeight <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float SafeMinimumSpeed = FMath::Max(MinimumFlagSpinBallSpeed, 0.0f);
	const float SafeFullSpeed = FMath::Max(FullFlagSpinBallSpeed, SafeMinimumSpeed + 1.0f);
	const float SpeedAlpha = FMath::Clamp(
		(BallSpeed - SafeMinimumSpeed) / (SafeFullSpeed - SafeMinimumSpeed),
		0.0f,
		1.0f);
	const float SafeMinimumAngularSpeed = FMath::Max(MinimumFlagSpinAngularSpeed, 0.0f);
	const float SafeMaximumAngularSpeed = FMath::Max(
		MaximumFlagSpinAngularSpeed,
		SafeMinimumAngularSpeed);
	const float ReactionAngularSpeed = FMath::Lerp(
		SafeMinimumAngularSpeed,
		SafeMaximumAngularSpeed,
		SpeedAlpha);
	const FVector CurrentAngularVelocity =
		FlagVisualMesh->GetPhysicsAngularVelocityInRadians(FlagBoneName);
	const float CurrentSpinSpeed = FVector::DotProduct(CurrentAngularVelocity, SpinAxis);
	const float DirectionMultiplier = bReverseFlagSpinDirection ? -1.0f : 1.0f;
	const float ReactionSpinSpeed = FMath::Sign(SignedSpinOffset)
		* DirectionMultiplier
		* ReactionAngularSpeed
		* SpinWeight;
	const float NewSpinSpeed = FMath::Clamp(
		CurrentSpinSpeed + ReactionSpinSpeed,
		-SafeMaximumAngularSpeed,
		SafeMaximumAngularSpeed);
	LastFlagSpinAngularVelocity = SpinAxis * NewSpinSpeed;
	FlagVisualMesh->SetPhysicsAngularVelocityInRadians(
		LastFlagSpinAngularVelocity,
		false,
		FlagBoneName);
	FlagVisualMesh->WakeAllRigidBodies();
}

bool APBGateBumperTriggerActor::MeetsMinimumPassSpeed(AActor* InteractionActor) const
{
	if (MinimumPassSpeed <= 0.0f)
	{
		return true;
	}

	const IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(InteractionActor);
	if (!Movable)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate pass speed could not be checked. Trigger=%s Target=%s"),
			*GetNameSafe(this),
			*GetNameSafe(InteractionActor));
		return false;
	}

	const float BallSpeed = Movable->GetVelocity().Size2D();
	return FMath::IsFinite(BallSpeed) && BallSpeed >= MinimumPassSpeed;
}

void APBGateBumperTriggerActor::HandleTriggerProgressChanged(
	const int32 CurrentCount,
	const int32 RequiredCount)
{
	CurrentGaugeAmount = RequiredCount > 0
		? FMath::Clamp(static_cast<float>(CurrentCount) / RequiredCount, 0.0f, 1.0f)
		: 0.0f;

	if (IsValid(GaugeMaterial))
	{
		GaugeMaterial->SetScalarParameterValue(GaugeParameterName, CurrentGaugeAmount);
	}
}

void APBGateBumperTriggerActor::HandleGateBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor)
		|| !PBInterfaceUtils::FindInterface<IMovable>(OtherActor))
	{
		return;
	}

	if (CanReactToMovableActor() && RegisterFlagSpinActorOverlap(OtherActor))
	{
		ApplyFlagSpinReaction(OtherActor, OverlappedComponent);
	}

	if (!CanIncreaseTrigger())
	{
		return;
	}

	const TWeakObjectPtr<AActor> ActorKey = OtherActor;
	int32& OverlapCount = PassingActorOverlapCounts.FindOrAdd(ActorKey);
	++OverlapCount;
}

void APBGateBumperTriggerActor::HandleGateEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	UnregisterFlagSpinActorOverlap(OtherActor);

	const TWeakObjectPtr<AActor> ActorKey = OtherActor;
	int32* OverlapCount = PassingActorOverlapCounts.Find(ActorKey);
	if (OverlapCount == nullptr)
	{
		return;
	}

	--(*OverlapCount);
	if (*OverlapCount > 0)
	{
		return;
	}

	PassingActorOverlapCounts.Remove(ActorKey);
	if (!CanIncreaseTrigger() || !MeetsMinimumPassSpeed(OtherActor))
	{
		return;
	}

	IncreaseTrigger(OtherActor, FHitResult());
}
