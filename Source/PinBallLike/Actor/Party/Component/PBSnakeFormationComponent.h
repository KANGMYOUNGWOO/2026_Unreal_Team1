// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBSnakeFormationComponent.generated.h"

class APBBallBase;

USTRUCT()
struct FPBSnakeTrailSample
{
	GENERATED_BODY()

	FVector Location = FVector::ZeroVector;
	float TimeSeconds = 0.0f;
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
	bool FindTrailLocation(float TargetTime, FVector& OutLocation) const;
	void TrimTrail(float CurrentTime);

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.01"))
	float FollowerDelaySeconds = 0.12f;

	UPROPERTY(EditAnywhere, Category = "Party|Snake", meta = (ClampMin = "0.01"))
	float TrailKeepSeconds = 1.0f;

	UPROPERTY()
	TArray<TObjectPtr<APBBallBase>> PartyBalls;

	TArray<FPBSnakeTrailSample> LeaderTrail;
};
