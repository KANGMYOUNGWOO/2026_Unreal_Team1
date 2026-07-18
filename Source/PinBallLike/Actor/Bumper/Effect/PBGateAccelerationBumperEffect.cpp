#include "PBGateAccelerationBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateAccelerationField.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"

UPBGateAccelerationBumperEffect::UPBGateAccelerationBumperEffect()
{
	SummonActorClass = APBGateAccelerationField::StaticClass();
	IsDestroyOnFinished = false;
}

void UPBGateAccelerationBumperEffect::Initialize(APBModularBumperBase* InOwnerBumper)
{
	Super::Initialize(InOwnerBumper);
	ConfigureAccelerationField();
}

void UPBGateAccelerationBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration skipped because Power is invalid. Power=%.2f"),
			EffectData.Power);
		FinishEffect();
		return;
	}

	if (!IsValid(Bumper) || !IsValid(InteractionActor) || !EnsureSummonActor(Bumper))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration activation context is invalid. Bumper=%s Target=%s Field=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*GetNameSafe(SpawnedSummonActor));
		FinishEffect();
		return;
	}

	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	if (!IsValid(ActiveTrigger))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration skipped because the active Trigger is missing. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	if (APBGateAccelerationField* AccelerationField = Cast<APBGateAccelerationField>(SpawnedSummonActor))
	{
		AccelerationField->SetDebugTriggerOrigin(ActiveTrigger->GetActorLocation());
	}
	if (!ConfigureAccelerationField())
	{
		FinishEffect();
		return;
	}

	const float ResolvedDuration = EffectData.Duration > 0.0f
		? EffectData.Duration
		: FieldDuration;
	PlayResolvedVfx(SpawnedSummonActor, ResolvedDuration, true);

	Super::ActivateEffectForActor(Bumper, InteractionActor);
}

FTransform UPBGateAccelerationBumperEffect::ResolveSpawnTransform(
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

bool UPBGateAccelerationBumperEffect::ConfigureAccelerationField() const
{
	APBGateAccelerationField* AccelerationField = Cast<APBGateAccelerationField>(SpawnedSummonActor);
	if (!IsValid(AccelerationField))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Gate acceleration field is unavailable. Effect=%s SummonActor=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SpawnedSummonActor));
		return false;
	}

	const float ResolvedDuration = EffectData.Duration > 0.0f
		? EffectData.Duration
		: FieldDuration;
	AccelerationField->ConfigureField(EffectData.Power, ResolvedDuration);
	return true;
}
