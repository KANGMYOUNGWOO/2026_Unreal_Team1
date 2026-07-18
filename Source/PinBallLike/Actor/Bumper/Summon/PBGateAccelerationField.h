#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PBGateAccelerationField.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateAccelerationField : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBGateAccelerationField();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

	void ConfigureField(float InSpeedBoostPercent, float InActiveDuration);

	UFUNCTION(BlueprintPure, Category = "Bumper|Gate Field")
	float GetFieldRadius() const { return FieldRadius; }

	void SetDebugTriggerOrigin(const FVector& InTriggerOrigin);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Gate Field")
	TObjectPtr<UBoxComponent> FieldArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Gate Field")
	TObjectPtr<USphereComponent> RadialFieldArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Gate Field")
	TObjectPtr<UStaticMeshComponent> FieldVisual;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Gate Field")
	float SpeedBoostPercent = 25.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Gate Field")
	float ActiveDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Gate Field",
		meta = (ClampMin = "50.0", ClampMax = "800.0", Units = "cm"))
	float FieldRadius = PBGateFieldTuning::DefaultRadius;

private:
	TMap<TWeakObjectPtr<AActor>, int32> OverlappingActorCounts;
	TSet<TWeakObjectPtr<AActor>> AcceleratedActors;
	FTimerHandle ActiveDurationTimerHandle;
	FVector DebugTriggerOrigin = FVector::ZeroVector;
	bool bHasDebugTriggerOrigin = false;

	void SetFieldActive(bool bIsActive);
	void RefreshFieldGeometry();
	bool ApplyAcceleration(AActor* InteractionActor) const;
	void HandleActiveDurationFinished();

	UFUNCTION()
	void HandleFieldBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleFieldEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
