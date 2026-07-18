// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PBGateAccelerationBumperEffect.generated.h"

/**
 * Effect 시트의 Power를 가속률로 사용해 Gate 가속 영역을 일정 시간 활성화한다.
 * 영역 생성과 종료 수명주기는 기존 Summon Effect 흐름을 재사용한다.
 */
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

	/** 영역 지속시간. 가속률은 Effect 시트의 Power에서 읽는다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect|Gate", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float FieldDuration = 5.0f;

private:
	bool ConfigureAccelerationField() const;
};
