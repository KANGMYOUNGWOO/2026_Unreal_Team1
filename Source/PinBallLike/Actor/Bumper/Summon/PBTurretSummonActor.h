// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PBTurretSummonActor.generated.h"

class APBBallBase;
class APBModularBumperBase;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBTurretSummonActor : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBTurretSummonActor();

	/** Effect 시트의 Power를 각 포탑 탄환에 전달합니다. */
	void SetAttackPayload(EPBBumperProjectilePayload InPayload, int32 InPower);

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

protected:
#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon|Turret")
	void OnTurretActivatedForActor(APBModularBumperBase* Bumper, AActor* InteractionActor);

	/** 기존 Ball 타입 Blueprint 이벤트 핀 호환을 위해 유지한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon|Turret")
	void OnTurretActivated(APBModularBumperBase* Bumper, APBBallBase* Ball);

#pragma endregion

private:
	AActor* FindBossTarget() const;

	EPBBumperProjectilePayload AttackPayload = EPBBumperProjectilePayload::None;
	int32 AttackPower = 0;
};
