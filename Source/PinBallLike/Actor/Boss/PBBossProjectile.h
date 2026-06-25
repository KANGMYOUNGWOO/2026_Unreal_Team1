#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectKey.h"
#include "PBBossProjectile.generated.h"

class APBBallBase;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PINBALLLIKE_API APBBossProjectile : public AActor
{
	GENERATED_BODY()

public:
	APBBossProjectile();

	UFUNCTION(BlueprintCallable, Category = "Boss|Projectile")
	void SetProjectileSpeed(float NewProjectileSpeed);

	UFUNCTION(BlueprintPure, Category = "Boss|Projectile")
	float GetProjectileSpeed() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleProjectileBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0.1"))
	float LifeTimeSeconds = 3.0f;

private:
	void ApplyProjectileSpeed();
	void SetProjectileCollision();
	void CheckInitialOverlappingPinballs();
	void ProcessPinballOverlap(APBBallBase* Pinball);

	TSet<TObjectKey<APBBallBase>> ProcessedPinballs;
};
