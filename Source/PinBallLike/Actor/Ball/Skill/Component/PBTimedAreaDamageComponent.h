#pragma once

#include "CoreMinimal.h"
#include "PBDamageComponentBase.h"
#include "TimerManager.h"
#include "PBTimedAreaDamageComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBTimedAreaDamageFinishedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FPBTimedAreaDamageAppliedSignature,
	AActor*, Target,
	int32, DamageAmount,
	FVector, HitLocation);

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBTimedAreaDamageComponent : public UPBDamageComponentBase
{
	GENERATED_BODY()

public:
	UPBTimedAreaDamageComponent();

	void InitializeDamageArea(
		UPrimitiveComponent* InDamageArea,
		AActor* InIgnoredActor);

	void ConfigureDamage(int32 InDamageAmount, float InDuration, int32 InDamageCount);

	UFUNCTION(BlueprintCallable, Category = "Damage|Area")
	bool ActivateEffect();

	UFUNCTION(BlueprintCallable, Category = "Damage|Area")
	void DeactivateEffect();

	void UpdateOverlappingTargetDamage();

	UFUNCTION(BlueprintPure, Category = "Damage|Area")
	bool IsEffectActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure, Category = "Damage|Area")
	int32 GetDamageAmount() const { return DamageAmount; }

	UFUNCTION(BlueprintPure, Category = "Damage|Area")
	float GetDuration() const { return Duration; }

	UFUNCTION(BlueprintPure, Category = "Damage|Area")
	int32 GetDamageCount() const { return DamageCount; }

	UFUNCTION(BlueprintPure, Category = "Damage|Area")
	float GetDamageInterval() const { return DamageInterval; }

	UPROPERTY(BlueprintAssignable, Category = "Damage|Area")
	FPBTimedAreaDamageFinishedSignature OnEffectFinished;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Area")
	FPBTimedAreaDamageAppliedSignature OnAreaDamageApplied;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Area", meta = (ClampMin = "0.01"))
	float DamageCheckInterval = 0.05f;

private:
	struct FTargetDamageState
	{
		double NextDamageTime = 0.0;
		int32 AppliedHitCount = 0;
	};

	UFUNCTION()
	void HandleDamageAreaBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleDamageAreaEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	void AddOverlappingTarget(AActor* Target);
	FVector CalculateHitLocation(const AActor* Target) const;
	bool IsValidTarget(AActor* Target) const;
	void StartTimers();
	void StopTimers();
	void FinishEffect();
	void UnbindDamageArea();

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> DamageArea;

	TWeakObjectPtr<AActor> IgnoredActor;
	TMap<TWeakObjectPtr<AActor>, FTargetDamageState> TargetDamageStates;

	FTimerHandle DamageCheckTimerHandle;
	FTimerHandle DurationTimerHandle;

	int32 DamageAmount = 0;
	float Duration = 0.0f;
	int32 DamageCount = 0;
	float DamageInterval = 0.0f;
	bool bIsActive = false;
};
