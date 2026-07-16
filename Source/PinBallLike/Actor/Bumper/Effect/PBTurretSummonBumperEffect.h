#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PBTurretSummonBumperEffect.generated.h"

/**
 * 기존 포탑 소환 Blueprint에 Effect 시트의 Power를 탄환 피해로 전달합니다.
 * 포탑의 등장·연사·퇴장 순서는 Blueprint가, 표적과 명중 피해는 C++이 담당합니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBTurretSummonBumperEffect : public UPBSummonBumperEffect
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
