// Fill out your copyright notice in the Description page of Project Settings.

#include "PBComboBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

void UPBComboBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (!IsValid(InteractionActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Combo effect skipped because the target Actor is invalid. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(InteractionActor);
	if (!Comboable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Combo effect skipped because the target has no combo interface. Target=%s"),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Combo effect skipped because Power is invalid. Power=%.2f"),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const int32 ComboAmount = FMath::Max(FMath::RoundToInt(EffectData.Power), 1);
	Comboable->AddCombo(ComboAmount);
	UE_LOG(LogTemp, Log, TEXT("[Bumper] Combo effect applied. Bumper=%s Target=%s Amount=%d"),
		*GetNameSafe(Bumper),
		*GetNameSafe(InteractionActor),
		ComboAmount);

	// 즉시 효과는 성공 여부와 관계없이 범퍼를 Idle 상태로 돌려야 한다.
	FinishEffect();
}
