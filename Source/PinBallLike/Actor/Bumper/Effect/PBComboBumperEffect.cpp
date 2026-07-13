// Fill out your copyright notice in the Description page of Project Settings.

#include "PBComboBumperEffect.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

void UPBComboBumperEffect::ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	if (!IsValid(Ball))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Combo effect skipped because Ball is invalid. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(Ball);
	if (!Comboable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Combo effect skipped because Ball has no combo interface. Ball=%s"),
			*GetNameSafe(Ball));
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
	UE_LOG(LogTemp, Log, TEXT("[Bumper] Combo effect applied. Bumper=%s Ball=%s Amount=%d"),
		*GetNameSafe(Bumper),
		*GetNameSafe(Ball),
		ComboAmount);

	// 즉시 효과는 성공 여부와 관계없이 범퍼를 Idle 상태로 돌려야 한다.
	FinishEffect();
}
