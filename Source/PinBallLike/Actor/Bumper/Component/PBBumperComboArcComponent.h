#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperComboArcComponent.generated.h"

class APBBumperProjectile;
class APBModularBumperBase;
class UPBBallComboComponent;

UCLASS(Transient)
class PINBALLLIKE_API UPBBumperComboArcComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperComboArcComponent();

	bool Arm(
		APBModularBumperBase* SourceBumper,
		AActor* BossTarget,
		TSubclassOf<APBBumperProjectile> ProjectileClass,
		int32 ComboInterval,
		int32 Damage,
		float Duration,
		const FVector& SpawnOffset,
		float ProjectileLifetime);

	bool IsArmed() const { return bArmed; }
	int32 GetAccumulatedComboGain() const { return AccumulatedComboGain; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPBBallComboComponent* ResolveComboComponent();
	void Disarm();
	bool FireArcProjectile() const;

	UFUNCTION()
	void HandleComboChanged(int32 CurrentCombo, int32 MaxCombo);

	TWeakObjectPtr<UPBBallComboComponent> CachedComboComponent;
	TWeakObjectPtr<APBModularBumperBase> ArmedBumper;
	TWeakObjectPtr<AActor> ArmedBossTarget;
	TSubclassOf<APBBumperProjectile> ArmedProjectileClass;
	FVector ArmedSpawnOffset = FVector::ZeroVector;
	int32 ArmedComboInterval = 0;
	int32 ArmedDamage = 0;
	int32 LastCombo = 0;
	int32 AccumulatedComboGain = 0;
	float ArmedProjectileLifetime = 3.0f;
	bool bArmed = false;
	bool bDelegateBound = false;
	FTimerHandle ExpirationTimer;
};
