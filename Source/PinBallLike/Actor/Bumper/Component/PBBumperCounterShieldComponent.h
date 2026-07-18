#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperCounterShieldComponent.generated.h"

class APBBumperProjectile;
class APBModularBumperBase;
class UPBBaseResourceComponent;

UCLASS(Transient)
class PINBALLLIKE_API UPBBumperCounterShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperCounterShieldComponent();

	bool Arm(
		APBModularBumperBase* SourceBumper,
		const FTransform& SourceTransform,
		AActor* BossTarget,
		TSubclassOf<APBBumperProjectile> ProjectileClass,
		int32 Damage,
		float Duration,
		const FVector& SpawnOffset,
		float ProjectileLifetime);

	bool IsArmed() const { return bArmed; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
#if WITH_DEV_AUTOMATION_TESTS
	friend class FPBBumperCounterShieldSourceTransformTest;
#endif

	UPBBaseResourceComponent* ResolveResourceComponent();
	void Disarm();
	bool FireCounterProjectile() const;
	FVector ResolveProjectileSpawnLocation() const;
	void HandleResourceStructureChanged(FName ChangedResourceName);
	void HandleResourceCurrentChanged(FName ChangedResourceName, float NewValue);

	TWeakObjectPtr<UPBBaseResourceComponent> CachedResourceComponent;
	TWeakObjectPtr<APBModularBumperBase> ArmedBumper;
	TWeakObjectPtr<AActor> ArmedBossTarget;
	TSubclassOf<APBBumperProjectile> ArmedProjectileClass;
	FTransform ArmedSourceTransform = FTransform::Identity;
	FVector ArmedSpawnOffset = FVector::ZeroVector;
	int32 ArmedDamage = 0;
	float ArmedProjectileLifetime = 3.0f;
	float LastShieldValue = 0.0f;
	bool bArmed = false;
	bool bDelegatesBound = false;
	FTimerHandle ExpirationTimer;
};
