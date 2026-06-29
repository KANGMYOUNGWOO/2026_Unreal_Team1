// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBumperSplineMeshActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

UENUM(BlueprintType)
enum class EPBBumperSplineMeshBuildMode : uint8
{
	StretchPerSegment,
	TileByLength
};

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperSplineMeshActor : public AActor
{
	GENERATED_BODY()

public:
	APBBumperSplineMeshActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Spline")
	void RebuildSplineMeshes();

	UFUNCTION(BlueprintPure, Category = "Bumper|Spline")
	USplineComponent* GetSplineComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Spline")
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Spline")
	TObjectPtr<USplineMeshComponent> SplineMeshTemplateComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Spline")
	TObjectPtr<UStaticMesh> SplineStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Spline")
	EPBBumperSplineMeshBuildMode BuildMode = EPBBumperSplineMeshBuildMode::StretchPerSegment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Spline|Tile", meta = (ClampMin = "1.0"))
	float TileLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Spline|Tile", meta = (ClampMin = "1"))
	int32 MaxTileCount = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Spline|Tile", meta = (ClampMin = "0.0"))
	float TileTangentScale = 1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Spline")
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshComponents;

private:
	void ClearSplineMeshes();
	void BuildStretchPerSegment();
	void BuildTileByLength();
	void SetupSplineMeshSegment(
		USplineMeshComponent* SplineMeshComponent,
		const FVector& StartLocation,
		const FVector& StartTangent,
		const FVector& EndLocation,
		const FVector& EndTangent);
	void GetSplinePointSegment(
		int32 StartPointIndex,
		FVector& OutStartLocation,
		FVector& OutStartTangent,
		FVector& OutEndLocation,
		FVector& OutEndTangent) const;
	void GetSplineDistanceSegment(
		float StartDistance,
		float EndDistance,
		FVector& OutStartLocation,
		FVector& OutStartTangent,
		FVector& OutEndLocation,
		FVector& OutEndTangent) const;
	USplineMeshComponent* CreateSplineMeshComponent(int32 MeshIndex);
};
