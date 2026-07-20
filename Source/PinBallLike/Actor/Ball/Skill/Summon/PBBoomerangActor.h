#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBoomerangActor.generated.h"

class APBBallBase;
class UPrimitiveComponent;
class UPBBallPhysicsComponent;
class USphereComponent;
class UPBInstantDamageComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBoomerangFinishedSignature,
	AActor*, EffectActor);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBoomerangActor : public AActor
{
	GENERATED_BODY()

public:
	APBBoomerangActor();

	void InitializeEffect(
		APBBallBase* InOwnerBall,
		int32 InDamageAmount,
		int32 InGroggyAmount,
		float InMaxPhaseDuration,
		int32 InMaxBounceCount);

	void ActivateEffect(FVector Direction);

	UPROPERTY(BlueprintAssignable, Category = "Boomerang")
	FPBBoomerangFinishedSignature OnEffectFinished;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boomerang")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boomerang")
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boomerang")
	TObjectPtr<UPBInstantDamageComponent> DamageComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boomerang", meta = (ClampMin = "0.0"))
	float MovementSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boomerang", meta = (ClampMin = "0.0"))
	float ReturnDistance = 60.0f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Boomerang")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boomerang")
	void OnHit(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boomerang")
	void OnBounce(int32 BounceCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boomerang")
	void OnReturning();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boomerang")
	void OnFinished();

private:
	UFUNCTION()
	void HandleMovementHit(const FHitResult& Hit);

	UFUNCTION()
	void HandleDamageBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleDamageEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	void BeginReturn();
	void HandlePhaseTimeout();
	void ApplyDamageToTarget(AActor* Target);
	void FinishEffect();
	void ClearPhaseTimer();

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> OwnerBall;

	TSet<TWeakObjectPtr<AActor>> OverlappingTargets;
	FTimerHandle PhaseTimerHandle;
	int32 DamageAmount = 0;
	int32 MaxBounceCount = 1;
	int32 CurrentBounceCount = 0;
	float MaxPhaseDuration = 1.0f;
	bool bReturning = false;
	bool bFinished = false;
};
