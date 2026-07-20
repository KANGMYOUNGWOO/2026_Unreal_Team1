#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallSkillProjectileFireComponent.generated.h"

class APBBallSkillProjectileBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBallSkillProjectileChangedSignature,
	AActor*, Projectile);

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallSkillProjectileFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallSkillProjectileFireComponent();

	void InitializeProjectileFire(AActor* InDamageSource, int32 InDamageAmount, int32 InGroggyAmount);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	AActor* FireOnce(FVector FireDirection);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ReleaseProjectile(AActor* Projectile);

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FPBBallSkillProjectileChangedSignature OnProjectileFired;

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FPBBallSkillProjectileChangedSignature OnProjectileActivated;

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FPBBallSkillProjectileChangedSignature OnProjectileDeactivated;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<APBBallSkillProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FName MuzzleTag = TEXT("SkillMuzzle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0.0"))
	float ProjectileLifeTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bUseObjectPool = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0"))
	int32 InitialPoolSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "1"))
	int32 MaxPoolSize = 20;

private:
	FTransform GetMuzzleTransform() const;
	APBBallSkillProjectileBase* GetProjectileFromPool();
	APBBallSkillProjectileBase* SpawnProjectileActor();
	void ActivateProjectile(APBBallSkillProjectileBase* Projectile, const FTransform& SpawnTransform);
	void DeactivateProjectile(APBBallSkillProjectileBase* Projectile);
	void CompleteProjectileDeactivation(APBBallSkillProjectileBase* Projectile);
	void ClearProjectiles();

	UFUNCTION()
	void HandleProjectileReleaseRequested(AActor* Projectile);

	UFUNCTION()
	void HandleProjectileDeactivationCompleted(AActor* Projectile);

	UPROPERTY(Transient)
	TObjectPtr<AActor> DamageSource;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBallSkillProjectileBase>> PooledProjectiles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBallSkillProjectileBase>> ActiveProjectiles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBallSkillProjectileBase>> DeactivatingProjectiles;

	TMap<TWeakObjectPtr<APBBallSkillProjectileBase>, FTimerHandle> ProjectileLifeTimerHandles;
	int32 DamageAmount = 0;
	int32 GroggyAmount = 0;
};
