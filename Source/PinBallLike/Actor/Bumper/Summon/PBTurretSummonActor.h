// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PBTurretSummonActor.generated.h"

class APBBallBase;
class APBModularBumperBase;
class UNiagaraSystem;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBTurretSummonActor : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBTurretSummonActor();

	void SetAttackPayload(
		EPBBumperProjectilePayload InPayload,
		int32 InPower,
		int32 InShotCount,
		UNiagaraSystem* InDeliveryVfx = nullptr,
		UNiagaraSystem* InImpactVfx = nullptr);

	UFUNCTION(BlueprintPure, Category = "Bumper|Summon|Turret")
	int32 GetAttackShotCount() const { return AttackShotCount; }

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

protected:
#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon|Turret")
	void OnTurretActivatedForActor(APBModularBumperBase* Bumper, AActor* InteractionActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon|Turret")
	void OnTurretActivated(APBModularBumperBase* Bumper, APBBallBase* Ball);

#pragma endregion

private:
	AActor* FindBossTarget() const;

	EPBBumperProjectilePayload AttackPayload = EPBBumperProjectilePayload::None;
	int32 AttackPower = 0;
	int32 AttackShotCount = 0;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> DeliveryVfx;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> ImpactVfx;
};
