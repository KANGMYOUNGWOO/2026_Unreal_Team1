// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBSummonBumperEffect.generated.h"

class APBBumperSummonActor;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBSummonBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(APBModularBumperBase* InOwnerBumper) override;
	virtual void ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball) override;
	virtual void FinishEffect() override;
	virtual void BeginDestroy() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	TSubclassOf<APBBumperSummonActor> SummonActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	FTransform SpawnOffset = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	bool IsDestroyOnFinished = false;

	UPROPERTY()
	TObjectPtr<APBBumperSummonActor> SpawnedSummonActor;

private:
	UFUNCTION()
	void HandleSummonActionFinished(APBBumperSummonActor* SummonActor);

	bool EnsureSummonActor(APBModularBumperBase* Bumper);
	void DeactivateSummonActor() const;
	void DestroySummonActor();
};
