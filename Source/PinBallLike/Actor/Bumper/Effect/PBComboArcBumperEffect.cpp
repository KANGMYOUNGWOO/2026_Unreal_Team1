#include "PBComboArcBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Component/PBBumperComboArcComponent.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"

void UPBComboArcBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static const TArray<FName> RequiredParameters =
	{
		TEXT("ComboInterval"),
		TEXT("DamageAmount"),
		TEXT("Duration")
	};

	FPBBumperSharedEffectDefinition Definition;
	FString ResolveError;
	int32 ComboInterval = 0;
	int32 DamageAmount = 0;
	float Duration = 0.0f;
	AActor* BossTarget = FindBossTarget(Bumper);
	const bool bResolved = PBBumperSharedEffectAdapter::Resolve(
		Bumper,
		EffectData.SharedEffectId,
		TEXT("ComboExtraDamage"),
		TEXT("Battle"),
		TEXT("All"),
		RequiredParameters,
		Definition,
		ResolveError)
		&& Definition.TryGetInt(TEXT("ComboInterval"), ComboInterval)
		&& Definition.TryGetInt(TEXT("DamageAmount"), DamageAmount)
		&& Definition.TryGetFloat(TEXT("Duration"), Duration)
		&& ComboInterval > 0
		&& DamageAmount > 0
		&& Duration > 0.0f;
	if (!bResolved
		|| !IsValid(InteractionActor)
		|| !IsValid(BossTarget)
		|| !ProjectileClass)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Bumper] Combo arc rejected. Bumper=%s Target=%s Boss=%s Error=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*GetNameSafe(BossTarget),
			*ResolveError);
		FinishEffect();
		return;
	}

	UPBBumperComboArcComponent* ArcComponent =
		InteractionActor->FindComponentByClass<UPBBumperComboArcComponent>();
	if (!IsValid(ArcComponent))
	{
		ArcComponent = NewObject<UPBBumperComboArcComponent>(
			InteractionActor,
			UPBBumperComboArcComponent::StaticClass(),
			NAME_None,
			RF_Transient);
		if (IsValid(ArcComponent))
		{
			InteractionActor->AddInstanceComponent(ArcComponent);
			ArcComponent->RegisterComponent();
		}
	}

	const bool bArmed = IsValid(ArcComponent)
		&& ArcComponent->Arm(
			Bumper,
			BossTarget,
			ProjectileClass,
			ProjectileMesh,
			ComboInterval,
			DamageAmount,
			Duration,
			ProjectileSpawnOffset,
			ProjectileLifetime);
	if (bArmed)
	{
		PlayStatusVfx(InteractionActor, Duration);
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Combo arc armed. Bumper=%s Target=%s Interval=%d Damage=%d Duration=%.1f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			ComboInterval,
			DamageAmount,
			Duration);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Combo arc component could not arm. Bumper=%s Target=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor));
	}
	FinishEffect();
}
