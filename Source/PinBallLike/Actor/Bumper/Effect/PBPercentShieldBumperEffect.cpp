#include "PBPercentShieldBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"

void UPBPercentShieldBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	UPBBaseResourceComponent* ResourceComponent = IsValid(InteractionActor)
		? InteractionActor->FindComponentByClass<UPBBaseResourceComponent>()
		: nullptr;
	if (!IsValid(Bumper)
		|| !IsValid(ResourceComponent)
		|| ResourceComponent->IsDead()
		|| !ResourceComponent->HasResource(PBResourceNames::Health)
		|| !FMath::IsFinite(EffectData.Power)
		|| EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Percent shield skipped. Bumper=%s Target=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const float MaxHealth = ResourceComponent->GetResourceMax(PBResourceNames::Health);
	const float ShieldCap = MaxHealth * FMath::Max(MaxShieldToHealthRatio, 0.0f);
	const float RequestedShield = MaxHealth * (FMath::Min(EffectData.Power, 1000.0f) / 100.0f);
	if (ShieldCap <= 0.0f || RequestedShield <= 0.0f)
	{
		FinishEffect();
		return;
	}

	if (!ResourceComponent->HasResource(PBResourceNames::Shield))
	{
		ResourceComponent->SetResource(PBResourceNames::Shield, 0.0f, ShieldCap);
	}
	else if (ResourceComponent->GetResourceMax(PBResourceNames::Shield) < ShieldCap)
	{
		ResourceComponent->SetResourceMax(PBResourceNames::Shield, ShieldCap, false);
	}

	const float PreviousShield = ResourceComponent->GetResourceCurrent(PBResourceNames::Shield);
	ResourceComponent->ApplyResourceDelta(PBResourceNames::Shield, RequestedShield);
	const float AppliedShield =
		ResourceComponent->GetResourceCurrent(PBResourceNames::Shield) - PreviousShield;
	if (AppliedShield > KINDA_SMALL_NUMBER)
	{
		PlayResolvedVfx(InteractionActor);
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Percent shield resolved. Bumper=%s Target=%s Percent=%.1f Requested=%.1f Applied=%.1f Cap=%.1f"),
		*GetNameSafe(Bumper),
		*GetNameSafe(InteractionActor),
		EffectData.Power,
		RequestedShield,
		AppliedShield,
		ResourceComponent->GetResourceMax(PBResourceNames::Shield));

	FinishEffect();
}
