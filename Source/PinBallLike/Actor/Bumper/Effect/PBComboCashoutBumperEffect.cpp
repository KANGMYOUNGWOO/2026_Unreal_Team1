#include "PBComboCashoutBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Projectile/PBBumperProjectile.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

namespace
{
	constexpr int32 MaxCashoutDamagePerActivation = 1000000;
}

void UPBComboCashoutBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(InteractionActor);
	AActor* BossTarget = FindBossTarget(Bumper);
	if (!Comboable
		|| !IsValid(BossTarget)
		|| !FMath::IsFinite(EffectData.Power)
		|| EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Combo cashout skipped. Bumper=%s Target=%s Boss=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*GetNameSafe(BossTarget),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const int32 CurrentCombo = Comboable->GetCombo();
	if (CurrentCombo <= 0)
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("[Bumper] Combo cashout had no combo to consume. Target=%s"),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	const int32 ResolvedMaxConsumption = EffectData.Count > 0
		? EffectData.Count
		: MaxComboConsumption;
	const int32 ConsumedCombo = FMath::Min(
		CurrentCombo,
		FMath::Max(ResolvedMaxConsumption, 1));
	const double RawDamage = static_cast<double>(ConsumedCombo) * static_cast<double>(EffectData.Power);
	const int32 DamageAmount = FMath::Clamp(
		FMath::RoundToInt(FMath::Min(RawDamage, static_cast<double>(MaxCashoutDamagePerActivation))),
		1,
		MaxCashoutDamagePerActivation);
	// 먼저 소비하고 탄환 생성이 실패하면 되돌려 한쪽만 적용되는 상태를 막습니다.
	const bool bComboReserved = Comboable->TryConsumeCombo(ConsumedCombo);
	const bool bProjectileSpawned = bComboReserved
		&& SpawnBossProjectile(
			Bumper,
			BossTarget,
			EPBBumperProjectilePayload::BossDamage,
			DamageAmount);
	if (bComboReserved && !bProjectileSpawned)
	{
		Comboable->AddCombo(ConsumedCombo);
	}
	const bool bComboCommitted = bComboReserved && bProjectileSpawned;

	if (bProjectileSpawned && bComboCommitted)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Combo cashout projectile spawned. Bumper=%s Boss=%s CurrentCombo=%d ConsumedCombo=%d RemainingCombo=%d Multiplier=%.2f Damage=%d"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BossTarget),
			CurrentCombo,
			ConsumedCombo,
			Comboable->GetCombo(),
			EffectData.Power,
			DamageAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Combo cashout did not fire. Bumper=%s Boss=%s ProjectileSpawned=%s ComboConsumed=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BossTarget),
			bProjectileSpawned ? TEXT("true") : TEXT("false"),
			bComboCommitted ? TEXT("true") : TEXT("false"));
	}

	FinishEffect();
}
