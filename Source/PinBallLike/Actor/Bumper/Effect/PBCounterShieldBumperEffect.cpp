#include "PBCounterShieldBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Component/PBBumperCounterShieldComponent.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"

void UPBCounterShieldBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	UPBBaseResourceComponent* ResourceComponent = IsValid(InteractionActor)
		? InteractionActor->FindComponentByClass<UPBBaseResourceComponent>()
		: nullptr;
	AActor* BossTarget = FindBossTarget(Bumper);
	const float ShieldAmount = EffectData.Power;
	const int32 CounterDamage = FMath::RoundToInt(EffectData.SecondaryPower);
	const float CounterDuration = EffectData.Duration;
	if (!IsValid(Bumper)
		|| !IsValid(ResourceComponent)
		|| ResourceComponent->IsDead()
		|| !ResourceComponent->HasResource(PBResourceNames::Health)
		|| !IsValid(BossTarget)
		|| !ProjectileClass
		|| !FMath::IsFinite(ShieldAmount)
		|| ShieldAmount <= 0.0f
		|| CounterDamage <= 0
		|| !FMath::IsFinite(CounterDuration)
		|| CounterDuration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Counter shield rejected. Bumper=%s Target=%s Boss=%s Shield=%.2f Damage=%d Duration=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*GetNameSafe(BossTarget),
			ShieldAmount,
			CounterDamage,
			CounterDuration);
		FinishEffect();
		return;
	}

	const float ShieldCap = ResourceComponent->GetResourceMax(PBResourceNames::Health)
		* FMath::Max(MaxShieldToHealthRatio, 0.0f);
	if (!ResourceComponent->HasResource(PBResourceNames::Shield))
	{
		ResourceComponent->SetResource(PBResourceNames::Shield, 0.0f, ShieldCap);
	}
	else if (ResourceComponent->GetResourceMax(PBResourceNames::Shield) < ShieldCap)
	{
		ResourceComponent->SetResourceMax(PBResourceNames::Shield, ShieldCap, false);
	}

	const float PreviousShield = ResourceComponent->GetResourceCurrent(PBResourceNames::Shield);
	ResourceComponent->ApplyResourceDelta(PBResourceNames::Shield, ShieldAmount);
	const float AppliedShield =
		ResourceComponent->GetResourceCurrent(PBResourceNames::Shield) - PreviousShield;
	UPBBumperCounterShieldComponent* CounterComponent =
		InteractionActor->FindComponentByClass<UPBBumperCounterShieldComponent>();
	if (!IsValid(CounterComponent))
	{
		CounterComponent = NewObject<UPBBumperCounterShieldComponent>(
			InteractionActor,
			UPBBumperCounterShieldComponent::StaticClass(),
			NAME_None,
			RF_Transient);
		if (IsValid(CounterComponent))
		{
			InteractionActor->AddInstanceComponent(CounterComponent);
			CounterComponent->RegisterComponent();
		}
	}

	const bool bArmed = AppliedShield > KINDA_SMALL_NUMBER
		&& IsValid(CounterComponent)
		&& CounterComponent->Arm(
			Bumper,
			BossTarget,
			ProjectileClass,
			CounterDamage,
			CounterDuration,
			ProjectileSpawnOffset,
			ProjectileLifetime);
	if (bArmed)
	{
		PlayResolvedVfx(InteractionActor, CounterDuration, true);
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Counter shield resolved. Bumper=%s Target=%s ShieldApplied=%.2f Armed=true"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			AppliedShield);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Counter shield could not arm. Bumper=%s Target=%s ShieldApplied=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			AppliedShield);
	}
	FinishEffect();
}
