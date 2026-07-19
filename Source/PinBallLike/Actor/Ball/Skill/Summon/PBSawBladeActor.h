#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBSawBladeActor.generated.h"

class APBBallBase;
class UPBBallPhysicsComponent;
class USphereComponent;
class UPBTimedAreaDamageComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBSawBladeFinishedSignature,
	AActor*, EffectActor);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBSawBladeActor : public AActor
{
	GENERATED_BODY()

public:
	APBSawBladeActor();

	void InitializeEffect(
		APBBallBase* InOwnerBall,
		int32 InDamageAmount,
		int32 InGroggyAmount,
		float InDuration,
		int32 InMaxBounceCount);

	void ActivateEffect(FVector Direction);

	UPROPERTY(BlueprintAssignable, Category = "SawBlade")
	FPBSawBladeFinishedSignature OnEffectFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SawBlade")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SawBlade")
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SawBlade")
	TObjectPtr<UPBTimedAreaDamageComponent> DamageComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SawBlade", meta = (ClampMin = "0.0"))
	float MovementSpeed = 1600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SawBlade", meta = (ClampMin = "0.01"))
	float DamageInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SawBlade", meta = (ClampMin = "1"))
	int32 MaxBounceCount = 10;

	UFUNCTION(BlueprintImplementableEvent, Category = "SawBlade")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "SawBlade")
	void OnHit(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "SawBlade")
	void OnBounce();

	UFUNCTION(BlueprintImplementableEvent, Category = "SawBlade")
	void OnFinished();

private:
	UFUNCTION()
	void HandleMovementHit(const FHitResult& Hit);

	UFUNCTION()
	void HandleDamageApplied(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION()
	void HandleDamageFinished();

	void FinishEffect();

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> OwnerBall;

	int32 DamageAmount = 0;
	int32 CurrentBounceCount = 0;
	float Duration = 0.0f;
	bool bFinished = false;
};
