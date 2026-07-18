// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PBGateAccelerationBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBGateAccelerationBumperEffect : public UPBSummonBumperEffect
{
	GENERATED_BODY()

public:
	UPBGateAccelerationBumperEffect();

	virtual void Initialize(APBModularBumperBase* InOwnerBumper) override;
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	virtual FTransform ResolveSpawnTransform(
		APBModularBumperBase* Bumper,
		bool& bOutUsesSummonAnchor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Gate", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float FieldDuration = 5.0f;

private:
	bool ConfigureAccelerationField() const;
};
