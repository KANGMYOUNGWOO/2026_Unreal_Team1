// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PBBumperEffectBase.generated.h"

class APBBallBase;
class APBModularBumperBase;
class AActor;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBumperEffectBase : public UObject
{
	GENERATED_BODY()

public:
	void InitializeEffect(
		APBModularBumperBase* InOwnerBumper,
		const FPBBumperEffectRow& InEffectData);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void Initialize(APBModularBumperBase* InOwnerBumper);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void ActivateEffectForActor(APBModularBumperBase* Bumper, AActor* InteractionActor);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void FinishEffect();

	virtual void ShutdownEffect();

	UFUNCTION(BlueprintPure, Category = "Bumper|Effect")
	FPBBumperEffectRow GetEffectData() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Effect")
	static AActor* ResolveBallEffectTargetOrSource(AActor* InteractionActor);

protected:
	void PlayDeliveryVfx(AActor* TargetActor, float Duration = 0.55f) const;
	void PlayImpactVfx(AActor* TargetActor) const;
	void PlayStatusVfx(AActor* TargetActor, float Duration) const;
	void StopStatusVfx(AActor* TargetActor) const;
	void PlayResolvedVfx(AActor* TargetActor, float StatusDuration = 0.0f, bool bPlayStatus = false) const;
	FName MakeVfxChannel(const TCHAR* Stage) const;

	UPROPERTY()
	TWeakObjectPtr<APBModularBumperBase> OwnerBumper;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	FPBBumperEffectRow EffectData;
};
