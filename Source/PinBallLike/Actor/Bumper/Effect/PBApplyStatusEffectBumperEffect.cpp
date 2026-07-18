// Fill out your copyright notice in the Description page of Project Settings.

#include "PBApplyStatusEffectBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"

namespace
{
	constexpr int32 MaxStatusEffectApplicationsPerActivation = 100;
}

void UPBApplyStatusEffectBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (!IsValid(Bumper) || !IsValid(InteractionActor))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Status effect skipped because activation context is invalid. Bumper=%s Target=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	if (StatusEffectId.IsNone())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Status effect skipped because StatusEffectId is none. Effect=%s"),
			*GetNameSafe(this));
		FinishEffect();
		return;
	}

	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Status effect skipped because Power is invalid. StatusEffectId=%s Power=%.2f"),
			*StatusEffectId.ToString(),
			EffectData.Power);
		FinishEffect();
		return;
	}

	UPBStatusEffectComponent* StatusEffectComponent =
		InteractionActor->FindComponentByClass<UPBStatusEffectComponent>();
	if (!IsValid(StatusEffectComponent))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Status effect skipped because the target has no status effect component. Target=%s"),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	const float SafeApplicationPower = FMath::Min(
		EffectData.Power,
		static_cast<float>(MaxStatusEffectApplicationsPerActivation));
	const int32 ApplicationCount = FMath::Clamp(
		FMath::RoundToInt(SafeApplicationPower),
		1,
		MaxStatusEffectApplicationsPerActivation);

	if (EffectData.Power > static_cast<float>(MaxStatusEffectApplicationsPerActivation))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Status effect application count was limited. StatusEffectId=%s RequestedPower=%.2f AppliedLimit=%d"),
			*StatusEffectId.ToString(),
			EffectData.Power,
			ApplicationCount);
	}

	int32 SuccessfulApplicationCount = 0;
	for (int32 Index = 0; Index < ApplicationCount; ++Index)
	{
		if (!StatusEffectComponent->ApplyStatusEffect(StatusEffectId))
		{
			break;
		}

		++SuccessfulApplicationCount;
	}

	if (SuccessfulApplicationCount == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Status effect application failed. Bumper=%s Target=%s StatusEffectId=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*StatusEffectId.ToString());
	}
	else
	{
		const float VfxDuration = EffectData.Duration > 0.0f ? EffectData.Duration : 1.2f;
		PlayResolvedVfx(InteractionActor, VfxDuration, true);
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Status effect applied. Bumper=%s Target=%s StatusEffectId=%s Applied=%d Stack=%d"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*StatusEffectId.ToString(),
			SuccessfulApplicationCount,
			StatusEffectComponent->GetStatusEffectStackCount(StatusEffectId));
	}

	// 상태이상의 수명은 Ball의 StatusEffectComponent가 관리하므로 범퍼 효과는 즉시 종료한다.
	FinishEffect();
}
