#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBombActor.generated.h"

class APBBallBase;
class UPrimitiveComponent;
class UPBBallPhysicsComponent;
class UPBInstantDamageComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBombFinishedSignature,
	AActor*, BombActor);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBombActor : public AActor
{
	GENERATED_BODY()

public:
	APBBombActor();

	void InitializeBomb(APBBallBase* InOwnerBall, int32 InDamageAmount, int32 InGroggyAmount);
	void ActivateBomb(FVector InitialVelocity);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void Explode();

	UPROPERTY(BlueprintAssignable, Category = "Bomb")
	FPBBombFinishedSignature OnBombFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<USphereComponent> ExplosionArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<UPBInstantDamageComponent> DamageComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.01"))
	float ExplosionTime = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.0"))
	float EarlyExplosionRemainingTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.0"))
	float StopDuration = 1.0f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb")
	void RequestExplode();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb")
	void OnHit(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb")
	void OnExploded();

private:
	UFUNCTION()
	void HandleExplosionAreaBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void EnableEarlyExplosion();
	void BeginExplosion();
	void ApplyExplosionDamage();
	void ClearExplosionTimers();
	bool IsBossInExplosionArea() const;

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> OwnerBall;

	FTimerHandle EarlyExplosionTimerHandle;
	FTimerHandle ExplosionTimerHandle;
	int32 DamageAmount = 0;
	bool bCanEarlyExplode = false;
	bool bExplosionRequested = false;
	bool bExploded = false;
};
