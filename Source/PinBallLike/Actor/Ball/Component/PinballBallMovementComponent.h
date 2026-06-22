#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinballBallMovementComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallMovementHit, const FHitResult&, Hit);

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPinballBallMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPinballBallMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void SetVelocity(FVector NewVelocity);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void AddVelocity(FVector VelocityToAdd);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void Launch(FVector Direction, float Strength);

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "PinBall|Movement")
	void MoveWithSweep(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Movement")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Movement", meta = (ClampMin = "0.0"))
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Movement", meta = (ClampMin = "0.0"))
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Movement", meta = (ClampMin = "0.0"))
	float BounceDamping = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Movement", meta = (ClampMin = "1"))
	int32 MaxBounceCountPerTick = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Movement", meta = (ClampMin = "0.0"))
	float XGravity = 980.0f;

	UPROPERTY(BlueprintAssignable, Category = "PinBall|Movement")
	FOnBallMovementHit OnBallMovementHit;

private:
	UPrimitiveComponent* ResolveCollisionComponent();
	float CalculateImpactDamping(FVector IncomingDirection, FVector ImpactNormal) const;
	void ClampVelocityToSpeedRange();

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> CollisionComponent;
};
