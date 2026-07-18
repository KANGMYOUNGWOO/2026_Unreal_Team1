#include "PBPickupSpawnBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperPickupActor.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"

UPBPickupSpawnBumperEffect::UPBPickupSpawnBumperEffect()
{
	SummonActorClass = APBBumperPickupActor::StaticClass();
	IsDestroyOnFinished = false;
}

void UPBPickupSpawnBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (!IsValid(Bumper)
		|| !IsValid(InteractionActor)
		|| !FMath::IsFinite(EffectData.Power)
		|| EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Pickup spawn skipped. Bumper=%s Target=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			EffectData.Power);
		FinishEffect();
		return;
	}

	APBBumperPickupActor* PickupActor = Cast<APBBumperPickupActor>(SpawnedSummonActor);
	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	if (!IsValid(PickupActor) || !IsValid(ActiveTrigger))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Pickup spawn skipped because its runtime assets are missing. Pickup=%s Trigger=%s"),
			*GetNameSafe(PickupActor),
			*GetNameSafe(ActiveTrigger));
		FinishEffect();
		return;
	}

	FTransform PickupTransform = SpawnOffset * ActiveTrigger->GetActorTransform();
	PickupTransform.AddToTranslation(ActiveTrigger->GetActorForwardVector() * SpawnDistance);
	PickupActor->SetActorTransform(PickupTransform);
	PickupActor->ConfigurePickup(
		RewardType,
		ResourceName,
		StatusEffectId,
		EffectData.Power,
		PickupLifetime,
		PickupColor);
	PlayResolvedVfx(PickupActor, PickupLifetime, true);

	Super::ActivateEffectForActor(Bumper, InteractionActor);
}
