#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallDashComponent.generated.h"

class UPBBallPhysicsComponent;
class AActor;

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallDashComponent();

	void InitializeDependencies(UPBBallPhysicsComponent* InPhysicsComponent);

	UFUNCTION(BlueprintCallable, Category = "Ball|Dash")
	bool DashInDirection(FVector Direction, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Ball|Dash")
	bool DashToBoss(float Speed);

	UFUNCTION(BlueprintCallable, Category = "Ball|Dash")
	bool DashToBossWithDefaultSpeed();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Dash", meta = (ClampMin = "0.0"))
	float DefaultDashSpeed = 2500.0f;

private:
	AActor* FindBossTarget() const;
	FVector ResolveDashDirectionToActor(const AActor* TargetActor) const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;
};
