#include "PBGateSupportFieldBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
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
	if (!IsValid(Bumper)
		|| !IsValid(InteractionActor)
		|| !FMath::IsFinite(EffectData.Power)
		|| EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Support field skipped. Bumper=%s Target=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			EffectData.Power);
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

	SupportField->SetActorTransform(SpawnOffset * ActiveTrigger->GetActorTransform());
	SupportField->ConfigureField(
		RewardType,
		ResourceName,
		StatusEffectId,
		EffectData.Power,
		FieldDuration,
		FieldColor);

	Super::ActivateEffectForActor(Bumper, InteractionActor);
}
