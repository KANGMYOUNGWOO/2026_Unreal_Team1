// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBVelocityBoostBumperEffect.generated.h"

/**
 * 충돌 프레임의 반사 및 속도 제한이 끝난 뒤 대상 Actor의 최종 진행 방향으로 순간 가속을 예약한다.
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
	/** 시트 오입력으로 한 프레임 이동량이 과도해지는 것을 막는 추가 속도 상한이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Velocity", meta = (ClampMin = "0.0"))
	float MaxBoostPower = 3000.0f;
};
