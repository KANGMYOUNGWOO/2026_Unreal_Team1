#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallDashComponent.generated.h"

class UPBBallPhysicsComponent;
class AActor;
class USoundBase;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Dash")
	TObjectPtr<USoundBase> DashSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Dash", meta = (ClampMin = "0.0"))
	float DashSoundVolume = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Dash", meta = (ClampMin = "0.0"))
	float DashSoundStartTime = 0.0f;

private:
	AActor* FindBossTarget() const;
	FVector ResolveDashDirectionToActor(const AActor* TargetActor) const;
	void PlayDashSound() const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;
};
