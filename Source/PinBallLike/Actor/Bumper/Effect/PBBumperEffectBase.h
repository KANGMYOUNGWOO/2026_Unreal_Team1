// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBBumperEffectBase.generated.h"

class APBBallBase;
class APBModularBumperBase;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBumperEffectBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void Initialize(APBModularBumperBase* InOwnerBumper);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void FinishEffect();

protected:
	UPROPERTY()
	TWeakObjectPtr<APBModularBumperBase> OwnerBumper;
};
