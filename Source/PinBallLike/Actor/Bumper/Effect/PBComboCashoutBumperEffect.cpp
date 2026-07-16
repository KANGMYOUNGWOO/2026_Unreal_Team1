#include "PBComboCashoutBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Interface/BossInterface.h"
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

	const double RawDamage = static_cast<double>(CurrentCombo) * static_cast<double>(EffectData.Power);
	const int32 DamageAmount = FMath::Clamp(
		FMath::RoundToInt(FMath::Min(RawDamage, static_cast<double>(MaxCashoutDamagePerActivation))),
		1,
		MaxCashoutDamagePerActivation);
	// 먼저 소비하고 피해 적용이 실패하면 되돌려 한쪽만 적용되는 상태를 막습니다.
	const bool bComboReserved = Comboable->TryConsumeCombo(CurrentCombo);
	const bool bDamageApplied = bComboReserved
		&& IBossInterface::Execute_DamageToBoss(BossTarget, DamageAmount);
	if (bComboReserved && !bDamageApplied)
	{
		Comboable->AddCombo(CurrentCombo);
	}
	const bool bComboCommitted = bComboReserved && bDamageApplied;

	if (bDamageApplied && bComboCommitted)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Combo cashout completed. Bumper=%s Boss=%s Combo=%d Multiplier=%.2f Damage=%d"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BossTarget),
			CurrentCombo,
			EffectData.Power,
			DamageAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Combo cashout did not complete. Bumper=%s Boss=%s DamageApplied=%s ComboConsumed=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BossTarget),
			bDamageApplied ? TEXT("true") : TEXT("false"),
			bComboCommitted ? TEXT("true") : TEXT("false"));
	}

	FinishEffect();
}
