#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallPhysicsComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallMovementHit, const FHitResult&, Hit);

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallPhysicsComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void SetVelocity(FVector NewVelocity);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	FVector GetVelocity() const;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	void SetMass(float NewMass);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	float GetMass() const;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	void SetBounceDamping(float NewBounceDamping);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	float GetBounceDamping() const;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void AddVelocity(FVector VelocityToAdd);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void AddImpulse(FVector Impulse);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void Launch(FVector Direction, float Strength);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void PauseMovement();

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void ResumeMovement();

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	bool IsMovementPaused() const;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void MoveWithSweep(float DeltaTime);

	UPROPERTY(BlueprintAssignable, Category = "PinBall|Movement")
	FOnBallMovementHit OnBallMovementHit;

private:
	UPrimitiveComponent* ResolveCollisionComponent();
	float CalculateImpactDamping(FVector IncomingDirection, FVector ImpactNormal) const;
	void ClampVelocityToSpeedRange();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Movement", meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Physics", meta = (ClampMin = "0.001", AllowPrivateAccess = "true"))
	float Mass = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Movement", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Movement", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Physics", meta = (ClampMin = "0.0", ClampMax = "1.0", AllowPrivateAccess = "true"))
	float BounceDamping = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Movement", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
	int32 MaxBounceCountPerTick = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinBall|Movement", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float XGravity = 980.0f;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> CollisionComponent;

	bool bMovementPaused = false;
};
