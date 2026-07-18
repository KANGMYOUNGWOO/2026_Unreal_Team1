// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBVelocityBoostBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBVelocityBoostBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Velocity", meta = (ClampMin = "0.0"))
	float MaxBoostPower = 3000.0f;
};
