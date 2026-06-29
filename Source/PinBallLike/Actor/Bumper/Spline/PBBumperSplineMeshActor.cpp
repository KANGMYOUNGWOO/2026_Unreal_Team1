// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperSplineMeshActor.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

APBBumperSplineMeshActor::APBBumperSplineMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SetRootComponent(SplineComponent);

	SplineMeshTemplateComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMeshTemplate"));
	SplineMeshTemplateComponent->SetupAttachment(SplineComponent);

	// 템플릿은 설정 복사용이라 실제 화면에는 표시하지 않는다.
	SplineMeshTemplateComponent->SetVisibility(false);
	SplineMeshTemplateComponent->SetHiddenInGame(true);
	SplineMeshTemplateComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APBBumperSplineMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RebuildSplineMeshes();
}

void APBBumperSplineMeshActor::RebuildSplineMeshes()
{
	//기존 메쉬들을 정리하고 새로 만듦
	ClearSplineMeshes();

	if (!IsValid(SplineComponent) || !IsValid(SplineMeshTemplateComponent))
	{
		return;
	}

	const int32 SplinePointCount = SplineComponent->GetNumberOfSplinePoints();
	if (SplinePointCount < 2)
	{
		return;
	}

	switch (BuildMode)
	{
	case EPBBumperSplineMeshBuildMode::TileByLength:
		BuildTileByLength();
		break;
	case EPBBumperSplineMeshBuildMode::StretchPerSegment:
	default:
		BuildStretchPerSegment();
		break;
	}
}

USplineComponent* APBBumperSplineMeshActor::GetSplineComponent() const
{
	return SplineComponent;
}

void APBBumperSplineMeshActor::BuildStretchPerSegment()
{
	// Spline point 사이마다 하나의 SplineMesh를 길게 늘려 배치한다.
	const int32 SplinePointCount = SplineComponent->GetNumberOfSplinePoints();
	for (int32 SegmentIndex = 0; SegmentIndex < SplinePointCount - 1; ++SegmentIndex)
	{
		USplineMeshComponent* SplineMeshComponent = CreateSplineMeshComponent(SegmentIndex);
		if (!IsValid(SplineMeshComponent))
		{
			continue;
		}

		FVector StartLocation;
		FVector StartTangent;
		FVector EndLocation;
		FVector EndTangent;

		GetSplinePointSegment(
			SegmentIndex,
			StartLocation,
			StartTangent,
			EndLocation,
			EndTangent);

		SetupSplineMeshSegment(
			SplineMeshComponent,
			StartLocation,
			StartTangent,
			EndLocation,
			EndTangent);
	}
}

void APBBumperSplineMeshActor::BuildTileByLength()
{
	// 전체 spline 길이를 TileLength 기준으로 나눠 짧은 SplineMesh를 반복 배치한다.
	const float SplineLength = SplineComponent->GetSplineLength();
	if (SplineLength <= 0.0f || TileLength <= 0.0f)
	{
		return;
	}

	const int32 TileCount = FMath::Min(FMath::CeilToInt(SplineLength / TileLength), MaxTileCount);
	for (int32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
	{
		const float StartDistance = TileIndex * TileLength;
		const float EndDistance = FMath::Min(StartDistance + TileLength, SplineLength);
		if (EndDistance <= StartDistance)
		{
			continue;
		}

		USplineMeshComponent* SplineMeshComponent = CreateSplineMeshComponent(TileIndex);
		if (!IsValid(SplineMeshComponent))
		{
			continue;
		}

		FVector StartLocation;
		FVector StartTangent;
		FVector EndLocation;
		FVector EndTangent;

		GetSplineDistanceSegment(
			StartDistance,
			EndDistance,
			StartLocation,
			StartTangent,
			EndLocation,
			EndTangent);

		const float SegmentLength = EndDistance - StartDistance;

		// 짧은 타일에서 tangent가 과하면 메쉬가 찌그러져 구간 길이에 맞춰 보정한다.
		StartTangent = StartTangent.GetSafeNormal() * SegmentLength * TileTangentScale;
		EndTangent = EndTangent.GetSafeNormal() * SegmentLength * TileTangentScale;

		SetupSplineMeshSegment(
			SplineMeshComponent,
			StartLocation,
			StartTangent,
			EndLocation,
			EndTangent);
	}
}

void APBBumperSplineMeshActor::SetupSplineMeshSegment(
	USplineMeshComponent* SplineMeshComponent,
	const FVector& StartLocation,
	const FVector& StartTangent,
	const FVector& EndLocation,
	const FVector& EndTangent)
{
	if (!IsValid(SplineMeshComponent))
	{
		return;
	}

	if (IsValid(SplineStaticMesh))
	{
		SplineMeshComponent->SetStaticMesh(SplineStaticMesh);
	}

	SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);
	SplineMeshComponent->RegisterComponent();
	SplineMeshComponents.Add(SplineMeshComponent);
}

void APBBumperSplineMeshActor::GetSplinePointSegment(
	const int32 StartPointIndex,
	FVector& OutStartLocation,
	FVector& OutStartTangent,
	FVector& OutEndLocation,
	FVector& OutEndTangent) const
{
	SplineComponent->GetLocationAndTangentAtSplinePoint(
		StartPointIndex,
		OutStartLocation,
		OutStartTangent,
		ESplineCoordinateSpace::Local);
	SplineComponent->GetLocationAndTangentAtSplinePoint(
		StartPointIndex + 1,
		OutEndLocation,
		OutEndTangent,
		ESplineCoordinateSpace::Local);
}

void APBBumperSplineMeshActor::GetSplineDistanceSegment(
	const float StartDistance,
	const float EndDistance,
	FVector& OutStartLocation,
	FVector& OutStartTangent,
	FVector& OutEndLocation,
	FVector& OutEndTangent) const
{
	OutStartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(
		StartDistance,
		ESplineCoordinateSpace::Local);
	OutStartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(
		StartDistance,
		ESplineCoordinateSpace::Local);
	OutEndLocation = SplineComponent->GetLocationAtDistanceAlongSpline(
		EndDistance,
		ESplineCoordinateSpace::Local);
	OutEndTangent = SplineComponent->GetTangentAtDistanceAlongSpline(
		EndDistance,
		ESplineCoordinateSpace::Local);
}

void APBBumperSplineMeshActor::ClearSplineMeshes()
{
	for (USplineMeshComponent* SplineMeshComponent : SplineMeshComponents)
	{
		if (IsValid(SplineMeshComponent))
		{
			SplineMeshComponent->DestroyComponent();
		}
	}

	SplineMeshComponents.Reset();
}

USplineMeshComponent* APBBumperSplineMeshActor::CreateSplineMeshComponent(const int32 SegmentIndex)
{
	// BP에서 조정한 SplineMeshTemplate 설정을 복사해서 실제 구간 메쉬를 만든다.
	USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(
		this,
		USplineMeshComponent::StaticClass(),
		*FString::Printf(TEXT("SplineMesh_%d"), SegmentIndex),
		RF_Transactional,
		SplineMeshTemplateComponent);

	if (!IsValid(SplineMeshComponent))
	{
		return nullptr;
	}

	SplineMeshComponent->SetMobility(EComponentMobility::Movable);
	SplineMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	SplineMeshComponent->AttachToComponent(
		SplineComponent,
		FAttachmentTransformRules::KeepRelativeTransform);
	SplineMeshComponent->SetVisibility(true);
	SplineMeshComponent->SetHiddenInGame(false);
	AddInstanceComponent(SplineMeshComponent);

	return SplineMeshComponent;
}
