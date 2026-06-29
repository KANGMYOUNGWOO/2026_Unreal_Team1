// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PBTurretSummonActor.generated.h"

class APBBallBase;
class APBModularBumperBase;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBTurretSummonActor : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBTurretSummonActor();

	virtual void StartAction(APBModularBumperBase* Bumper, APBBallBase* Ball) override;
	virtual void DeactivateSummon() override;

protected:
#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Summon|Turret")
	void OnTurretActivated(APBModularBumperBase* Bumper, APBBallBase* Ball);

#pragma endregion
};
