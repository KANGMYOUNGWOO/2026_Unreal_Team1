#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Projectile/ProjectileBase.h"
#include "PBBallSkillProjectileBase.generated.h"

class APBBallSkillProjectileBase;
class UPBInstantDamageComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FPBBallSkillProjectileDamageAppliedSignature,
	AActor*, Target,
	int32, DamageAmount,
	FVector, HitLocation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBallSkillProjectileReleaseRequestedSignature,
	AActor*, Projectile);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBallSkillProjectileDeactivationCompletedSignature,
	AActor*, Projectile);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBallSkillProjectileBase : public AProjectileBase
{
	GENERATED_BODY()

public:
	APBBallSkillProjectileBase();

	void InitializeProjectile(int32 InDamageAmount, int32 InGroggyAmount);
	virtual void ActivateProjectile() override;
	virtual void DeactivateProjectile() override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void CompleteDeactivation();

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FPBBallSkillProjectileDamageAppliedSignature OnProjectileDamageApplied;

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FPBBallSkillProjectileReleaseRequestedSignature OnProjectileReleaseRequested;

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FPBBallSkillProjectileDeactivationCompletedSignature OnProjectileDeactivationCompleted;

protected:
	static const FName SkillCollisionProfileName;

	virtual void HandleProjectileBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UPBInstantDamageComponent> DamageComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bReleaseOnFirstDamage = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bWaitForVisualCompletion = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnHit(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnDeactivated();

private:
	int32 DamageAmount = 0;
	bool bIsDeactivating = false;
	bool bDeactivationCompleted = false;
};
