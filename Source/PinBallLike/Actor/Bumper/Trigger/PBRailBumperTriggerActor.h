// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PBRailBumperTriggerActor.generated.h"

class APBBallBase;
class APBBumperSplineMeshActor;
class IMovable;
class UPrimitiveComponent;
class USplineComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBRailBumperTriggerActor : public APBBumperTriggerActorBase
{
	GENERATED_BODY()

public:
	APBRailBumperTriggerActor();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Rail")
	void FinishRailMove();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Rail")
	void OnRailMoveStarted(APBBallBase* Ball);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Rail")
	void OnRailMoveUpdated(APBBallBase* Ball, float Alpha);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Rail")
	void OnRailMoveFinished(APBBallBase* Ball);
#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Rail")
	FName TriggerAreaTag = TEXT("BumperTrigger");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Rail")
	FName RailSplineTag = TEXT("RailSpline");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Rail")
	TObjectPtr<APBBumperSplineMeshActor> RailSplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Rail", meta = (ClampMin = "1.0", Units = "cm/s"))
	float RailMoveSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Rail")
	bool IsStopBallMovementOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Rail")
	bool IsPauseBallMovementOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Rail")
	bool IsResumeBallMovementOnFinish = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Rail", meta = (ClampMin = "0.0", Units = "cm/s"))
	float ExitVelocityStrength = 600.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Rail")
	TArray<TObjectPtr<UPrimitiveComponent>> TriggerAreas;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Rail")
	TObjectPtr<USplineComponent> RailSplineComponent;

private:
	UPROPERTY()
	TObjectPtr<APBBallBase> MovingBall;

	IMovable* MovingBallMovable = nullptr;
	float CurrentRailDistance = 0.0f;

	void RegisterRailAreas();
	void SetupTriggerArea(UPrimitiveComponent* TriggerArea);
	void ResolveRailSplineComponent();
	bool StartRailMove(APBBallBase* Ball);
	void AddExitVelocityToBall();
	bool IsRailMoving() const;

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);
};
