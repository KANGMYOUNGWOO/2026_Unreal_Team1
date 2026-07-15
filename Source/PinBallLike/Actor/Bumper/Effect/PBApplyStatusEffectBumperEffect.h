// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBApplyStatusEffectBumperEffect.generated.h"

/**
 * Effect 시트의 Power를 적용 횟수로 변환해 대상 Actor의 상태이상 컴포넌트에 전달한다.
 * 실제 효과 수치와 지속시간은 StatusEffect 데이터가 소유하며 이 클래스는 적용 요청만 담당한다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBApplyStatusEffectBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	/** 적용할 StatusEffect RowName. 파생 Effect Blueprint의 기본값에서 지정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|StatusEffect")
	FName StatusEffectId = NAME_None;
};
