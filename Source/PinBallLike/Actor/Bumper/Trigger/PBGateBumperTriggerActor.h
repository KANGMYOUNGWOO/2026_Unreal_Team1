// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PBGateBumperTriggerActor.generated.h"

class UMaterialInstanceDynamic;
class UMeshComponent;
class UPrimitiveComponent;
class USkeletalMeshComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateBumperTriggerActor : public APBBumperTriggerActorBase
{
	GENERATED_BODY()

public:
	APBGateBumperTriggerActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger", meta = (ClampMin = "0.0"))
	float MinimumPassSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	FName GateAreaTag = TEXT("BumperTrigger");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<TObjectPtr<UPrimitiveComponent>> GateAreas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual")
	FName GaugeVisualTag = TEXT("BumperVisual");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual")
	FName GaugeParameterName = TEXT("GaugeAmount");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual", meta = (ClampMin = "0"))
	int32 GaugeMaterialIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Visual")
	float CurrentGaugeAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	bool bEnableFlagSpinReaction = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagVisualTag = TEXT("GateFlagVisual");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagBoneName = TEXT("Flag");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagEndBoneName = TEXT("End");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FName FlagConstraintName = TEXT("Flag");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "cm/s"))
	float MinimumFlagSpinBallSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "cm/s"))
	float FullFlagSpinBallSpeed = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "rad/s"))
	float MinimumFlagSpinAngularSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0", Units = "rad/s"))
	float MaximumFlagSpinAngularSpeed = 34.906585f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.01"))
	float FlagFullSpinOffsetRatio = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag", meta = (ClampMin = "0.0"))
	float FlagSpinDamping = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	bool bReverseFlagSpinDirection = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	FVector LastFlagSpinAngularVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Visual|Flag")
	bool bIsFlagSpinReactionReady = false;

private:
	TMap<TWeakObjectPtr<AActor>, int32> PassingActorOverlapCounts;
	TMap<TWeakObjectPtr<AActor>, int32> FlagSpinActorOverlapCounts;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GaugeMaterial;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> FlagVisualMesh;

	void RegisterGateAreas();
	void SetupGateArea(UPrimitiveComponent* GateArea);
	void InitializeGaugeVisual();
	void InitializeFlagSpinReaction();
	USkeletalMeshComponent* FindFlagVisualMesh() const;
	FVector CalculateFlagSpinAxis() const;
	bool RegisterFlagSpinActorOverlap(AActor* InteractionActor);
	void UnregisterFlagSpinActorOverlap(AActor* InteractionActor);
	void ApplyFlagSpinReaction(AActor* InteractionActor, const UPrimitiveComponent* GateArea);
	bool MeetsMinimumPassSpeed(AActor* InteractionActor) const;

	UFUNCTION()
	void HandleTriggerProgressChanged(int32 CurrentCount, int32 RequiredCount);

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
