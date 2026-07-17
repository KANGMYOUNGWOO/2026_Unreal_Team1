#include "PBKineticShellBumperEffect.h"

#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"

void UPBKineticShellBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static const TArray<FName> RequiredParameters =
	{
		TEXT("BaseDamage"),
		TEXT("SpeedPerBonus"),
		TEXT("MaxDamage")
	};

	FPBBumperSharedEffectDefinition Definition;
	FString ResolveError;
	float BaseDamage = 0.0f;
	float SpeedPerBonus = 0.0f;
	float MaxDamage = 0.0f;
	UPBBallPhysicsComponent* PhysicsComponent = IsValid(InteractionActor)
		? InteractionActor->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
	AActor* BossTarget = FindBossTarget(Bumper);
	const bool bResolved = PBBumperSharedEffectAdapter::Resolve(
		Bumper,
		EffectData.SharedEffectId,
		TEXT("VelocityScaledDamage"),
		RequiredParameters,
		Definition,
		ResolveError)
		&& Definition.TryGetFloat(TEXT("BaseDamage"), BaseDamage)
		&& Definition.TryGetFloat(TEXT("SpeedPerBonus"), SpeedPerBonus)
		&& Definition.TryGetFloat(TEXT("MaxDamage"), MaxDamage)
		&& BaseDamage > 0.0f
		&& SpeedPerBonus > 0.0f
		&& MaxDamage >= BaseDamage;

	if (!bResolved || !IsValid(PhysicsComponent) || !IsValid(BossTarget))
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Bumper] Kinetic shell rejected. Bumper=%s Target=%s Boss=%s Error=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*GetNameSafe(BossTarget),
			*ResolveError);
		FinishEffect();
		return;
	}

	const float BallSpeed = PhysicsComponent->GetVelocity().Size2D();
	const int32 DamageAmount = FMath::Clamp(
		FMath::FloorToInt(BaseDamage + BallSpeed / SpeedPerBonus),
		FMath::CeilToInt(BaseDamage),
		FMath::FloorToInt(MaxDamage));
	const bool bSpawned = SpawnBossProjectile(
		Bumper,
		BossTarget,
		EPBBumperProjectilePayload::BossDamage,
		DamageAmount);

	if (bSpawned)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Kinetic shell resolved. Bumper=%s Speed=%.1f Damage=%d Spawned=true"),
			*GetNameSafe(Bumper),
			BallSpeed,
			DamageAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Kinetic shell failed to spawn. Bumper=%s Speed=%.1f Damage=%d"),
			*GetNameSafe(Bumper),
			BallSpeed,
			DamageAmount);
	}
	FinishEffect();
}
