#include "PBGateSupportFieldBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateSupportField.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"

UPBGateSupportFieldBumperEffect::UPBGateSupportFieldBumperEffect()
{
	SummonActorClass = APBGateSupportField::StaticClass();
	IsDestroyOnFinished = false;
}

void UPBGateSupportFieldBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	EPBBumperRewardType ResolvedRewardType = RewardType;
	FName ResolvedResourceName = ResourceName;
	FName ResolvedTimedStatName = TimedStatName;
	float ResolvedPower = EffectData.Power;
	float ResolvedDuration = EffectData.Duration > 0.0f
		? EffectData.Duration
		: FieldDuration;
	int32 ResolvedTriggerCount = EffectData.Count;

	if (!EffectData.SharedEffectId.IsNone())
	{
		FPBBumperSharedEffectDefinition Definition;
		FString ResolveError;
		if (!PBBumperSharedEffectAdapter::Resolve(
			Bumper,
			EffectData.SharedEffectId,
			NAME_None,
			TEXT("Ball"),
			TEXT("All"),
			TConstArrayView<FName>(),
			Definition,
			ResolveError))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[Bumper] Shared support field rejected. Effect=%s Error=%s"),
				*EffectData.SharedEffectId.ToString(),
				*ResolveError);
			FinishEffect();
			return;
		}

		if (Definition.EffectType == TEXT("PostDamageHeal"))
		{
			FName SheetResourceName;
			if (!Definition.TryGetName(TEXT("ResourceName"), SheetResourceName)
				|| !PBBumperSharedEffectAdapter::ResolveResourceName(
					SheetResourceName,
					ResolvedResourceName)
				|| !Definition.TryGetInt(TEXT("Count"), ResolvedTriggerCount)
				|| !Definition.TryGetFloat(TEXT("Value"), ResolvedPower)
				|| !Definition.TryGetFloat(TEXT("Duration"), ResolvedDuration))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[Bumper] PostDamageHeal parameters are invalid. Effect=%s"),
					*EffectData.SharedEffectId.ToString());
				FinishEffect();
				return;
			}
			ResolvedRewardType = EPBBumperRewardType::PostDamageHeal;
		}
		else if (Definition.EffectType == TEXT("StatBuff"))
		{
			FName SheetStatName;
			FName ModifyType;
			if (!Definition.TryGetName(TEXT("StatName"), SheetStatName)
				|| !PBBumperSharedEffectAdapter::ResolveStatName(
					SheetStatName,
					ResolvedTimedStatName)
				|| !Definition.TryGetName(TEXT("ModifyType"), ModifyType)
				|| ModifyType != TEXT("PercentAdd")
				|| !Definition.TryGetFloat(TEXT("Value"), ResolvedPower)
				|| !Definition.TryGetFloat(TEXT("Duration"), ResolvedDuration))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[Bumper] StatBuff parameters are invalid. Effect=%s"),
					*EffectData.SharedEffectId.ToString());
				FinishEffect();
				return;
			}
			ResolvedRewardType = EPBBumperRewardType::TimedStatPercent;
			ResolvedTriggerCount = 0;
		}
		else
		{
			UE_LOG(LogTemp, Error,
				TEXT("[Bumper] Shared effect type cannot drive a support field. Effect=%s Type=%s"),
				*EffectData.SharedEffectId.ToString(),
				*Definition.EffectType.ToString());
			FinishEffect();
			return;
		}
	}

	if (!IsValid(Bumper)
		|| !IsValid(InteractionActor)
		|| !FMath::IsFinite(ResolvedPower)
		|| ResolvedPower <= 0.0f
		|| !EnsureSummonActor(Bumper))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Support field skipped. Bumper=%s Target=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			ResolvedPower);
		FinishEffect();
		return;
	}

	APBGateSupportField* SupportField = Cast<APBGateSupportField>(SpawnedSummonActor);
	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	if (!IsValid(SupportField) || !IsValid(ActiveTrigger))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Support field skipped because its runtime assets are missing. Field=%s Trigger=%s"),
			*GetNameSafe(SupportField),
			*GetNameSafe(ActiveTrigger));
		FinishEffect();
		return;
	}

	SupportField->SetDebugTriggerOrigin(ActiveTrigger->GetActorLocation());
	SupportField->ConfigureField(
		ResolvedRewardType,
		ResolvedResourceName,
		StatusEffectId,
		TimedEffectSourceId,
		ResolvedTimedStatName,
		ResolvedPower,
		ResolvedTriggerCount,
		ResolvedDuration,
		FieldColor);
	PlayResolvedVfx(SupportField, ResolvedDuration, true);

	Super::ActivateEffectForActor(Bumper, InteractionActor);
}

FTransform UPBGateSupportFieldBumperEffect::ResolveSpawnTransform(
	APBModularBumperBase* Bumper,
	bool& bOutUsesSummonAnchor)
{
	if (IsValid(Bumper))
	{
		const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
		if (IsValid(ActiveTrigger))
		{
			bOutUsesSummonAnchor = false;
			return PBGateFieldTuning::MakeTransformAtGateOrigin(ActiveTrigger->GetActorTransform());
		}
	}

	return Super::ResolveSpawnTransform(Bumper, bOutUsesSummonAnchor);
}
