// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flipper.generated.h"

class ABallBase;
class UBoxComponent;
class USceneComponent;

UCLASS()
class PINBALLLIKE_API AFlipper : public AActor
{
	GENERATED_BODY()

public:
	AFlipper();

	UFUNCTION(BlueprintCallable, Category = "Flipper")
	void SetIsMove(bool bIsMove);

protected:
	virtual void BeginPlay() override;

	bool UpdateRotation(float DeltaTime);
	void UpdateFlipperForce(float DeltaTime);
	FRotator GetAppliedRotationOffset() const;
	float CalculateMotionAlpha() const;
	float GetFlipperLength() const;
	float CalculateDistanceAlpha(FVector WorldLocation) const;
	FVector CalculateForceDirection(float DistanceAlpha) const;
	void ApplyForceToBalls(float DeltaTime, float MotionAlpha);
	void DrawForceDirectionDebug() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flipper|Launch")
	TObjectPtr<UBoxComponent> BallDetectTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipper|Launch", meta = (ClampMin = "0.0", Units = "cm/s^2"))
	float LaunchPower = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flipper|Launch", meta = (ClampMin = "0.0"))
	float TipPowerMultiplier = 1.8f;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Flipper")
	TObjectPtr<USceneComponent> Pivot;

	UPROPERTY(EditAnywhere, Category = "Flipper")
	FRotator MoveUpRotationOffset = FRotator(0.0, 45.0, 0.0);

	UPROPERTY(EditAnywhere, Category = "Flipper", meta = (ClampMin = "0.0", Units = "DegreesPerSecond"))
	float RotationSpeed = 360.0f;

	FRotator DownedRotation;
	bool bIsMoveUp = false;
};
