#pragma once

#include "CoreMinimal.h"
#include "PBBossBase.h"
#include "SnakeBoss.generated.h"

class USkeletalMeshComponent;
class APBBossMoveArea;

USTRUCT(BlueprintType)
struct FPBSnakePathSample
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Path")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Path")
	float Distance = 0.0f;
};

UCLASS()
class PINBALLLIKE_API ASnakeBoss : public APBBossBase
{
	GENERATED_BODY()

public:
	ASnakeBoss();

	void SetSnakeChargePose(bool IsActive, const FVector& Direction, float BlendAlpha);

	UFUNCTION(BlueprintPure, Category = "Boss|Snake Animation")
	float GetSnakeAnimationSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Snake Animation")
	float GetSnakeAnimationTurnAmount() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Snake Animation")
	float GetSnakeAnimationMovePhase() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Snake Animation")
	bool IsSnakeAnimationMoving() const;

	const TArray<FVector>& GetSnakeSplinePoints() const;
	const TArray<FVector>& GetSnakeChargeHeadSplinePoints() const;
	bool IsSnakeChargePoseActive() const;
	float GetSnakeChargePoseAlpha() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void StartIdleState_Implementation() override;

	void InitializeMoveArea();
	void InitializeMoveDirection();
	void MoveHead(float DeltaTime);
	void UpdateSnakeAnimationData(float DeltaTime, const FVector& NextLocation, const FVector& PreviousLocation);
	void ResetSnakePath();
	void RecordSnakePathLocation(const FVector& Location);
	void UpdateSnakeSplinePoints();
	void UpdateSnakeChargeHeadSplinePoints();
	void TrimSnakePath();
	bool FindSnakePathLocationAtDistance(float Distance, FVector& OutLocation) const;
	void SelectNextPatrolTarget();
	FVector ClampLocationToPatrolArea(const FVector& SourceLocation) const;
	APBBossMoveArea* FindNearestMoveArea() const;
	bool IsInsideHeadExcludedArea(const FVector& SourceLocation) const;
	FVector GetPatrolCurveLocation(float Alpha) const;
	float CalculatePatrolCurveDistance() const;
	void FaceMovementDirection(const FVector& Direction);
	void DrawDebugSnake() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake")
	TObjectPtr<USkeletalMeshComponent> SnakeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float TurnSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "1.0"))
	float PatrolAcceptanceRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float HeadExcludeRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake")
	TObjectPtr<APBBossMoveArea> BossMoveArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float PatrolCurveOffsetScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "0.0"))
	float PatrolMinCurveControlDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float PatrolTargetForwardDot = -0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake")
	bool IsDrawDebugSnake = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake Path", meta = (ClampMin = "2"))
	int32 SnakeSplinePointCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake Path", meta = (ClampMin = "1.0"))
	float SnakeSplineLength = 1300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake Charge", meta = (ClampMin = "2"))
	int32 SnakeChargeHeadSplinePointCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake Charge", meta = (ClampMin = "1.0"))
	float SnakeChargeHeadSplineLength = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Snake Charge", meta = (ClampMin = "0.0"))
	float SnakeChargeHeadCurveOffsetScale = 0.25f;

private:
	FVector CurrentMoveDirection = FVector::ForwardVector;
	FVector SnakeChargeDirection = FVector::ForwardVector;
	FVector PatrolStartLocation = FVector::ZeroVector;
	FVector PatrolCurveControlLocation = FVector::ZeroVector;
	FVector PatrolCurveEndControlLocation = FVector::ZeroVector;
	FVector PatrolTargetLocation = FVector::ZeroVector;
	float PatrolCurveAlpha = 0.0f;
	float PatrolCurveDistance = 0.0f;
	float SnakeAnimationSpeed = 0.0f;
	float SnakeAnimationTurnAmount = 0.0f;
	float SnakeAnimationMovePhase = 0.0f;
	float SnakePathTotalDistance = 0.0f;
	float SnakeChargePoseAlpha = 0.0f;
	TArray<FPBSnakePathSample> SnakePathSamples;
	TArray<FVector> SnakeSplinePoints;
	TArray<FVector> SnakeChargeHeadSplinePoints;
	bool IsPatrolTargetValid = false;
	bool IsSnakeAnimationMoveActive = false;
	bool IsSnakeChargePoseActiveValue = false;
};
