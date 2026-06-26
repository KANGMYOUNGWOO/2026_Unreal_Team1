// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PBGateBumperTriggerActor.generated.h"

class APBBallBase;
class UPrimitiveComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateBumperTriggerActor : public APBBumperTriggerActorBase
{
	GENERATED_BODY()

public:
	APBGateBumperTriggerActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	FName GateAreaTag = TEXT("BumperTrigger");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<UPrimitiveComponent>> GateAreas;

private:
	TMap<TWeakObjectPtr<APBBallBase>, int32> PassingBallOverlapCounts;
	TMap<TWeakObjectPtr<APBBallBase>, float> PassingBallEntrySides;

	void RegisterGateAreas();
	void SetupGateArea(UPrimitiveComponent* GateArea);
	float CalculateGateSide(const UPrimitiveComponent* GateArea, const APBBallBase* Ball) const;

	UFUNCTION()
	void HandleGateBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleGateEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
