// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBSnakeFormationComponent.generated.h"

class APBBallBase;

UENUM(BlueprintType)
enum class EPBSnakeFormationMode : uint8
{
	TimeDelayWithDistanceClamp,
	FixedDistance
};

USTRUCT()
struct FPBSnakeTrailSample
{
	GENERATED_BODY()

	FVector Location = FVector::ZeroVector;
	float TimeSeconds = 0.0f;
	float DistanceAlongTrail = 0.0f;
};

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBSnakeFormationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBSnakeFormationComponent();

	UFUNCTION(BlueprintCallable, Category = "Party|Snake")
	void SetPartyBalls(const TArray<APBBallBase*>& OrderedBalls);

	UFUNCTION(BlueprintCallable, Category = "Party|Snake")
	void ClearFormation();

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RecordLeaderTrail(float CurrentTime);
	void UpdateFollowers(float CurrentTime);
	void UpdateFollowersByTimeDelay(float CurrentTime);
	void UpdateFollowersByFixedDistance();
	bool FindTrailLocationAtTime(float TargetTime, FVector& OutLocation) const;
	bool FindTrailLocationAtDistance(float TargetDistance, FVector& OutLocation) const;
	FVector ClampTargetDistanceFromPreviousBall(const FVector& PreviousBallLocation, const FVector& TargetLocation) const;
	void TrimTrail(float CurrentTime);

	UPROPERTY(EditAnywhere, Category = "Party|Snake")
	EPBSnakeFormationMode FormationMode = EPBSnakeFormationMode::FixedDistance;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.01"))
	float FollowerDelaySeconds = 0.12f;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.0"))
	float MinFollowerDistance = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.0"))
	float MaxFollowerDistance = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.01"))
	float FixedFollowerDistance = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.01"))
	float TrailKeepSeconds = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.0"))
	float TrailKeepDistance = 500.0f;

	UPROPERTY()
	TArray<TObjectPtr<APBBallBase>> PartyBalls;

	TArray<FPBSnakeTrailSample> LeaderTrail;
};
