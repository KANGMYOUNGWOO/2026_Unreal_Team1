#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/Movable.h"
#include "PBBallPhysicsComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallMovementHit, const FHitResult&, Hit);

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallPhysicsComponent : public UActorComponent, public IMovable
{
	GENERATED_BODY()

public:
	UPBBallPhysicsComponent();
	void InitializeDependencies(UPrimitiveComponent* InPrimitiveComponent);

	UPROPERTY(BlueprintAssignable, Category = "PinBall|Movement")
	FOnBallMovementHit OnBallMovementHit;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual FVector GetVelocity() const override;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual void AddVelocity(FVector VelocityToAdd) override;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual void AddImpulse(FVector Impulse) override;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual void StopMovement() override;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual void PauseMovement() override;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual void ResumeMovement() override;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	virtual bool IsMovementPaused() const override;
	
	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	void SetMass(float NewMass);
	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	float GetMass() const;
	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	void SetBounceDamping(float NewBounceDamping);
	UFUNCTION(BlueprintCallable, Category = "PinBall|Physics")
	float GetBounceDamping() const;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void SetVelocity(FVector NewVelocity);
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void Launch(FVector Direction, float Strength);
	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void MoveWithSweep(float DeltaTime);
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
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
	TObjectPtr<UPrimitiveComponent> PrimitiveComponent = nullptr;

	bool bMovementPaused = false;
};
