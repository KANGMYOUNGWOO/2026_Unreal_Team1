// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBComboBumperEffect.generated.h"

/**
 * Effect 시트의 Power만큼 발동 대상의 IComboable 콤보를 한 번 증가시킨다.
 * Trigger 횟수 관리는 Modular Bumper에 남기고, 보상 적용 책임만 이 클래스가 가진다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBComboBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
