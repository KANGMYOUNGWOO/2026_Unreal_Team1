#include "PBOctopusTentacleSlamTelegraph.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr double TelegraphPlaneSize = 100.0;
}

APBOctopusTentacleSlamTelegraph::APBOctopusTentacleSlamTelegraph()
{
	PrimaryActorTick.bCanEverTick = true;

	TelegraphMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TelegraphMesh"));
	TelegraphMesh->SetupAttachment(RootSceneComponent);
	TelegraphMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TelegraphMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		TelegraphMesh->SetStaticMesh(PlaneMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TelegraphMaterialFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (TelegraphMaterialFinder.Succeeded())
	{
		TelegraphMesh->SetMaterial(0, TelegraphMaterialFinder.Object);
	}
}

void APBOctopusTentacleSlamTelegraph::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	static_cast<void>(DeltaSeconds);
	UpdateTrackedTransform();
}

void APBOctopusTentacleSlamTelegraph::InitSlamTelegraph(
	USceneComponent* NewStartComponent,
	AActor* NewTargetActor,
	const double Length,
	const double Width,
	const float DurationSeconds)
{
	if (!IsValid(NewStartComponent) || !IsValid(NewTargetActor) || Length <= 0.0 || Width <= 0.0)
	{
		DestroyTelegraph();
		return;
	}

	StartComponent = NewStartComponent;
	TargetActor = NewTargetActor;
	TelegraphLength = Length;
	InitTelegraph(
		DurationSeconds,
		FVector(Length / TelegraphPlaneSize, Width / TelegraphPlaneSize, 1.0));

	UMaterialInstanceDynamic* TelegraphMaterial = TelegraphMesh->CreateDynamicMaterialInstance(0);
	if (TelegraphMaterial)
	{
		TelegraphMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
	}

	UpdateTrackedTransform();
}

void APBOctopusTentacleSlamTelegraph::UpdateTrackedTransform()
{
	if (!IsValid(StartComponent) || !IsValid(TargetActor))
	{
		return;
	}

	const FVector StartLocation = StartComponent->GetComponentLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	FVector Direction = TargetLocation - StartLocation;
	Direction.Z = 0.0;
	if (!Direction.Normalize())
	{
		return;
	}
	const FRotator TelegraphRotation = Direction.Rotation();
	FVector CenterLocation = StartLocation + Direction * (TelegraphLength * 0.5);
	CenterLocation.Z = StartLocation.Z;
	SetActorLocationAndRotation(CenterLocation, TelegraphRotation);
}
