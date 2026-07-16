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
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void FinishEffect() override;
	virtual void ShutdownEffect() override;
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

	/** 파생 Effect가 발동 전 소환 Actor를 구성할 수 있도록 생성 여부를 보장합니다. */
	bool EnsureSummonActor(APBModularBumperBase* Bumper);

private:
	UFUNCTION()
	void HandleSummonActionFinished(APBBumperSummonActor* SummonActor);

	void DeactivateSummonActor() const;
	void DestroySummonActor();
};
