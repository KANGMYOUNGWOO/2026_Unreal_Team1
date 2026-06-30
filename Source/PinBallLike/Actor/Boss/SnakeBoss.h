#pragma once

#include "CoreMinimal.h"
#include "PBBossBase.h"
#include "SnakeBoss.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

USTRUCT()
struct FPBSnakeBossTrailSample
{
	GENERATED_BODY()

	FVector Location = FVector::ZeroVector;
	float Distance = 0.0f;
};

UCLASS()
class PINBALLLIKE_API ASnakeBoss : public APBBossBase
{
	GENERATED_BODY()

public:
	ASnakeBoss();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Boss|Snake")
	void RebuildSnakeSegments();

	void FaceHeadDirection(const FVector& Direction);
	void CollapseBodyToHead();
	void PullBodyToHead(float Alpha);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void StartIdleState_Implementation() override;

	void InitializePatrolCenter();
	void InitializeMoveDirection();
	void RecoverCollapsedBody();
	void MoveHead(float DeltaTime);
	void SelectNextPatrolTarget();
	FVector ClampLocationToPatrolArea(const FVector& SourceLocation) const;
	bool IsInsideHeadMeshExcludedArea(const FVector& SourceLocation) const;
	FVector GetPatrolCurveLocation(float Alpha) const;
	float CalculatePatrolCurveDistance() const;
	void FaceMovementDirection(const FVector& Direction);
	void ResetSnakeTrail();
	void RecordHeadLocation();
	void UpdateBodySegments();
	void DrawDebugSnake() const;
	void TrimHeadTrail();

	bool FindTrailLocationAtDistance(float TargetDistance, FVector& OutLocation) const;

	void StartSnakePattern();
	void StopSnakePattern();
	void EnterSnakeGroggy();
	void ExitSnakeGroggy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake")
	TArray<TObjectPtr<UStaticMeshComponent>> BodySegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "1"))
	int32 SegmentCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "1.0"))
	float SegmentSpacing = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float TurnSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "1.0"))
	float PatrolAcceptanceRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake")
	FVector PatrolCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake")
	FVector PatrolAreaExtent = FVector(700.0f, 700.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float HeadMeshExcludeRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake")
	bool IsUseSpawnLocationAsPatrolCenter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float PatrolCurveOffsetScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake")
	bool IsDrawDebugSnake = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float ChargePullDelayPerSegment = 0.12f;

private:
	void ConfigureSnakeMeshComponent(UStaticMeshComponent* MeshComponent) const;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	TArray<FPBSnakeBossTrailSample> HeadTrail;
	TArray<FVector> SegmentTargetLocations;

	FVector CurrentMoveDirection = FVector::ForwardVector;
	FVector PatrolStartLocation = FVector::ZeroVector;
	FVector PatrolCurveControlLocation = FVector::ZeroVector;
	FVector PatrolCurveEndControlLocation = FVector::ZeroVector;
	FVector PatrolTargetLocation = FVector::ZeroVector;
	float TotalTrailDistance = 0.0f;
	float PatrolCurveAlpha = 0.0f;
	float PatrolCurveDistance = 0.0f;
	bool IsPatrolCenterInitialized = false;
	bool IsPatrolTargetValid = false;
	bool IsSnakeBodyCollapsed = false;
};
