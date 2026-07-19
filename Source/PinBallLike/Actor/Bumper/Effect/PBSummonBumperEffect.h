// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonAnchor.h"
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

	UFUNCTION(BlueprintPure, Category = "Bumper|Effect|Summon|Anchor")
	EPBBumperSummonAnchorType GetSpawnAnchorType() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	TSubclassOf<APBBumperSummonActor> SummonActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	FTransform SpawnOffset = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon|Anchor")
	EPBBumperSummonAnchorType SpawnAnchorType = EPBBumperSummonAnchorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	bool IsDestroyOnFinished = false;

	UPROPERTY()
	TObjectPtr<APBBumperSummonActor> SpawnedSummonActor;

	bool EnsureSummonActor(APBModularBumperBase* Bumper);

	virtual FTransform ResolveSpawnTransform(
		APBModularBumperBase* Bumper,
		bool& bOutUsesSummonAnchor);

private:
	UFUNCTION()
	void HandleSummonActionFinished(APBBumperSummonActor* SummonActor);

	EPBBumperPositionId ResolveSourcePositionId(const APBModularBumperBase* Bumper) const;
	void UpdateSummonActorTransform(
		APBModularBumperBase* Bumper,
		const FTransform& SpawnTransform,
		bool bUsesSummonAnchor) const;
	void DeactivateSummonActor() const;
	void DestroySummonActor();

	bool bHasReportedAnchorResolutionFailure = false;
};
