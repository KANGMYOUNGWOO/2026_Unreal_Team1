#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBGateSupportField.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateSupportField : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBGateSupportField();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

	void ConfigureField(
		EPBBumperRewardType InRewardType,
		FName InResourceName,
		FName InStatusEffectId,
		FName InTimedEffectSourceId,
		FName InTimedStatName,
		float InRewardPower,
		int32 InTriggerCount,
		float InActiveDuration,
		const FLinearColor& InFieldColor);

	UFUNCTION(BlueprintPure, Category = "Bumper|Support Field")
	float GetFieldRadius() const { return FieldRadius; }

	void SetDebugTriggerOrigin(const FVector& InTriggerOrigin);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UBoxComponent> FieldArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<USphereComponent> RadialFieldArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UStaticMeshComponent> FieldVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UPointLightComponent> FieldLight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	EPBBumperRewardType RewardType = EPBBumperRewardType::Resource;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName ResourceName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName StatusEffectId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName TimedEffectSourceId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName TimedStatName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	float RewardPower = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	int32 TriggerCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	float ActiveDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Support Field",
		meta = (ClampMin = "50.0", ClampMax = "800.0", Units = "cm"))
	float FieldRadius = PBGateFieldTuning::DefaultRadius;

private:
	TSet<TWeakObjectPtr<AActor>> RewardedActors;
	FTimerHandle ActiveDurationTimerHandle;
	FVector DebugTriggerOrigin = FVector::ZeroVector;
	FColor DebugFieldColor = FColor::Green;
	bool bHasDebugTriggerOrigin = false;

	void SetFieldActive(bool bNewActive);
	void RefreshFieldGeometry();
	bool TryApplyReward(AActor* TargetActor);
	void HandleActiveDurationFinished();

	UFUNCTION()
	void HandleFieldBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
