// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBVelocityBoostBumperEffect.generated.h"

/**
 * 충돌 프레임의 반사 및 속도 제한이 끝난 뒤 대상 Actor의 속도를 목표 최저 속도까지 보정한다.
 * 구체 Ball 구현은 수정하지 않고 프로젝트 공용 IMovable 계약만 사용한다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBVelocityBoostBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	/** 시트 오입력으로 목표 속도가 과도해지는 것을 막는 안전 상한입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Velocity", meta = (ClampMin = "0.0"))
	float MaxBoostPower = 3000.0f;
};
