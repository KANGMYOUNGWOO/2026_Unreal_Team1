// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonAnchor.h"
#include "PBSummonBumperEffect.generated.h"

class APBBumperSummonActor;

/** 범퍼 효과가 소환 Actor를 생성, 재사용, 정리하는 공통 수명주기를 제공합니다. */
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

	/** None preserves the legacy Bumper-relative placement path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon|Anchor")
	EPBBumperSummonAnchorType SpawnAnchorType = EPBBumperSummonAnchorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Summon")
	bool IsDestroyOnFinished = false;

	UPROPERTY()
	TObjectPtr<APBBumperSummonActor> SpawnedSummonActor;

	/** 파생 Effect가 발동 전 소환 Actor를 구성할 수 있도록 생성 여부를 보장합니다. */
	bool EnsureSummonActor(APBModularBumperBase* Bumper);

	/** 파생 Effect가 소환 위치만 바꾸고 공통 생성·재사용 수명주기는 유지할 수 있는 확장 지점입니다. */
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
