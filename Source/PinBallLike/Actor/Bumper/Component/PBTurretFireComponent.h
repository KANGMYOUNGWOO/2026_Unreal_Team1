// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PBTurretFireComponent.generated.h"

class AProjectileBase;
class UNiagaraSystem;
class USceneComponent;
class UStaticMesh;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBTurretProjectileSignature,
	AActor*, Projectile);

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBTurretFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBTurretFireComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Turret|Fire")
	AActor* FireOnce();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Turret|Aim")
	bool AimAtTarget();

	static bool TryResolveAimRotation(
		const FVector& SourceLocation,
		const FVector& TargetLocation,
		const FRotator& CurrentRotation,
		bool bYawOnly,
		float YawOffsetDegrees,
		FRotator& OutRotation);

	static bool TryResolveAimRotationFromDirections(
		const FVector& CurrentAimDirection,
		const FVector& TargetDirection,
		const FRotator& CurrentRotation,
		bool bYawOnly,
		float YawOffsetDegrees,
		FRotator& OutRotation);

	void ConfigureAttack(
		AActor* InTargetActor,
		EPBBumperProjectilePayload InPayload,
		int32 InPower,
		int32 InMaxShotCount,
		UNiagaraSystem* InDeliveryVfx = nullptr,
		UNiagaraSystem* InImpactVfx = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Turret|Pool")
	void ReleaseProjectile(AActor* Projectile);

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Turret|Fire")
	FPBTurretProjectileSignature OnTurretProjectileFired;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Turret|Fire")
	FPBTurretProjectileSignature OnTurretProjectileActivated;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|Turret|Fire")
	FPBTurretProjectileSignature OnTurretProjectileDeactivated;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Fire")
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Fire")
	TObjectPtr<UStaticMesh> ProjectileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Fire")
	FName MuzzleTag = TEXT("TurretMuzzle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Fire", meta = (ClampMin = "0.0"))
	float ProjectileLifeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Aim")
	bool bAimAtTargetBeforeFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Aim")
	bool bYawOnlyAim = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Aim")
	FName AimPivotTag = TEXT("TurretAimPivot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Aim", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float AimYawOffsetDegrees = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Pool")
	bool IsUseObjectPool = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Pool", meta = (ClampMin = "0"))
	int32 InitialPoolSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Turret|Pool", meta = (ClampMin = "1"))
	int32 MaxPoolSize = 20;

private:
	void HandleBumperProjectileResolved(APBBumperProjectile* Projectile, bool bApplied);
	USceneComponent* ResolveAimPivot();
	USceneComponent* ResolveMuzzleComponent() const;
	FTransform GetMuzzleTransform() const;
	AProjectileBase* GetProjectileFromPool();
	AProjectileBase* SpawnProjectileActor();
	void ActivateProjectile(AProjectileBase* Projectile, const FTransform& SpawnTransform);
	void DeactivateProjectile(AProjectileBase* Projectile);
	void ClearPool();

	UPROPERTY()
	TArray<TObjectPtr<AProjectileBase>> PooledProjectiles;

	UPROPERTY()
	TArray<TObjectPtr<AProjectileBase>> ActiveProjectiles;

	TMap<TWeakObjectPtr<AProjectileBase>, FTimerHandle> ProjectileLifeTimerHandles;

	TWeakObjectPtr<AActor> AttackTarget;
	TWeakObjectPtr<USceneComponent> CachedAimPivot;
	EPBBumperProjectilePayload AttackPayload = EPBBumperProjectilePayload::None;
	int32 AttackPower = 0;
	int32 MaxAttackShotCount = 0;
	int32 FiredAttackShotCount = 0;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> DeliveryVfx;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> ImpactVfx;
};
