#pragma once

#include "CoreMinimal.h"

namespace PBEffectTypes
{
	namespace EffectType
	{
		inline const FName StatusEffect(TEXT("StatusEffect"));
		inline const FName StatBuff(TEXT("StatBuff"));
		inline const FName ResourceBuff(TEXT("ResourceBuff"));
		inline const FName DamageTakenModifier(TEXT("DamageTakenModifier"));
		inline const FName DamageIgnore(TEXT("DamageIgnore"));
		inline const FName FirstAttack(TEXT("FirstAttack"));
		inline const FName ComboStatBuff(TEXT("ComboStatBuff"));
		inline const FName SkillDamageModifier(TEXT("SkillDamageModifier"));
		inline const FName OnHitStatStack(TEXT("OnHitStatStack"));
		inline const FName OnHitDamageIgnoreChance(TEXT("OnHitDamageIgnoreChance"));
		inline const FName ComboExtraDamage(TEXT("ComboExtraDamage"));
		inline const FName MultiBall(TEXT("MultiBall"));
		inline const FName ReviveOnZero(TEXT("ReviveOnZero"));
		inline const FName Pierce(TEXT("Pierce"));
		inline const FName ResourceCostStatBuff(TEXT("ResourceCostStatBuff"));
		inline const FName ShopPriceDiscount(TEXT("ShopPriceDiscount"));
		inline const FName ShopRerollDiscount(TEXT("ShopRerollDiscount"));
		inline const FName PostDamageHeal(TEXT("PostDamageHeal"));
		inline const FName SwitchCountBonus(TEXT("SwitchCountBonus"));
		inline const FName SwitchNextHitDamage(TEXT("SwitchNextHitDamage"));
		inline const FName SwitchElasticityBuff(TEXT("SwitchElasticityBuff"));
		inline const FName ReviveStatBuff(TEXT("ReviveStatBuff"));
		inline const FName FirstHitTimedStatBuff(TEXT("FirstHitTimedStatBuff"));
		inline const FName ComboPeriodicStatBuff(TEXT("ComboPeriodicStatBuff"));
	}

	namespace TargetType
	{
		inline const FName Ball(TEXT("Ball"));
		inline const FName PartyBall(TEXT("PartyBall"));
		inline const FName PartyBalls(TEXT("PartyBalls"));
		inline const FName Leader(TEXT("Leader"));
		inline const FName Follower(TEXT("Follower"));
		inline const FName Followers(TEXT("Followers"));
		inline const FName Player(TEXT("Player"));
		inline const FName Deck(TEXT("Deck"));
		inline const FName Battle(TEXT("Battle"));
		inline const FName Shop(TEXT("Shop"));
		inline const FName Reward(TEXT("Reward"));
	}

	namespace TargetFilter
	{
		inline const FName All(TEXT("All"));
	}

	namespace ParamKey
	{
		inline const FName StatusEffectId(TEXT("StatusEffectId"));
		inline const FName StatName(TEXT("StatName"));
		inline const FName ResourceName(TEXT("ResourceName"));
		inline const FName ModifyType(TEXT("ModifyType"));
		inline const FName ApplyTo(TEXT("ApplyTo"));
		inline const FName Value(TEXT("Value"));
		inline const FName AttackPercent(TEXT("AttackPercent"));
		inline const FName ExtraDamage(TEXT("ExtraDamage"));
		inline const FName RequiredCombo(TEXT("RequiredCombo"));
		inline const FName ChancePercent(TEXT("ChancePercent"));
		inline const FName IgnoreCount(TEXT("IgnoreCount"));
		inline const FName ComboInterval(TEXT("ComboInterval"));
		inline const FName DamageAmount(TEXT("DamageAmount"));
		inline const FName Count(TEXT("Count"));
		inline const FName DamageRetentionPercent(TEXT("DamageRetentionPercent"));
		inline const FName ReviveValue(TEXT("ReviveValue"));
		inline const FName ResourceCost(TEXT("ResourceCost"));
		inline const FName Duration(TEXT("Duration"));
	}

	namespace ModifyType
	{
		inline const FName Add(TEXT("Add"));
		inline const FName PercentAdd(TEXT("PercentAdd"));
		inline const FName Set(TEXT("Set"));
	}

	namespace ResourceApplyTo
	{
		inline const FName Current(TEXT("Current"));
		inline const FName Max(TEXT("Max"));
		inline const FName CurrentAndMax(TEXT("CurrentAndMax"));
	}
}
