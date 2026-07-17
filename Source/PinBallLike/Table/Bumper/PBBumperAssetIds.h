// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"

namespace PBBumperAssetIds
{
	// PrimaryAssetType values used by AssetManager.
	namespace Type
	{
		inline const FPrimaryAssetType BumperData(TEXT("BumperData"));
	}

	// RowNames for BumperTable
	namespace Bumper
	{
		inline const FName Rebound_PowerPush(TEXT("Rebound_PowerPush"));
		inline const FName Rebound_KineticShell(TEXT("Rebound_KineticShell"));
		inline const FName Rebound_ComboPulse(TEXT("Rebound_ComboPulse"));
		inline const FName Rebound_BloodOverdrive(TEXT("Rebound_BloodOverdrive"));
		inline const FName Rebound_GroggyHammer(TEXT("Rebound_GroggyHammer"));
		inline const FName Side_ShieldCharge(TEXT("Side_ShieldCharge"));
		inline const FName Side_ManaCharge(TEXT("Side_ManaCharge"));
		inline const FName Side_RepairPickup(TEXT("Side_RepairPickup"));
		inline const FName Side_CounterShield(TEXT("Side_CounterShield"));
		inline const FName Side_StrengthCharge(TEXT("Side_StrengthCharge"));
		inline const FName Top_ComboArc(TEXT("Top_ComboArc"));
		inline const FName Top_GroggyStrike(TEXT("Top_GroggyStrike"));
		inline const FName Top_DirectStrike(TEXT("Top_DirectStrike"));
		inline const FName Top_ComboCashout(TEXT("Top_ComboCashout"));
		inline const FName Top_VulnerabilityShell(TEXT("Top_VulnerabilityShell"));
		inline const FName Gate_SpeedUp(TEXT("Gate_SpeedUp"));
		inline const FName Gate_RecoveryField(TEXT("Gate_RecoveryField"));
		inline const FName Gate_ReactiveRepair(TEXT("Gate_ReactiveRepair"));
		inline const FName Gate_ManaReactor(TEXT("Gate_ManaReactor"));
		inline const FName Gate_LaunchCharge(TEXT("Gate_LaunchCharge"));
	}

	// RowNames for TriggerTable
	namespace Trigger
	{
		inline const FName Rebound_Trigger_PowerPush(TEXT("Rebound_Trigger_PowerPush"));
		inline const FName Rebound_Trigger_KineticShell(TEXT("Rebound_Trigger_KineticShell"));
		inline const FName Rebound_Trigger_ComboPulse(TEXT("Rebound_Trigger_ComboPulse"));
		inline const FName Rebound_Trigger_BloodOverdrive(TEXT("Rebound_Trigger_BloodOverdrive"));
		inline const FName Rebound_Trigger_GroggyHammer(TEXT("Rebound_Trigger_GroggyHammer"));
		inline const FName Side_Trigger_ShieldCharge(TEXT("Side_Trigger_ShieldCharge"));
		inline const FName Side_Trigger_ManaCharge(TEXT("Side_Trigger_ManaCharge"));
		inline const FName Side_Trigger_RepairPickup(TEXT("Side_Trigger_RepairPickup"));
		inline const FName Side_Trigger_CounterShield(TEXT("Side_Trigger_CounterShield"));
		inline const FName Side_Trigger_StrengthCharge(TEXT("Side_Trigger_StrengthCharge"));
		inline const FName Top_Trigger_ComboArc(TEXT("Top_Trigger_ComboArc"));
		inline const FName Top_Trigger_GroggyStrike(TEXT("Top_Trigger_GroggyStrike"));
		inline const FName Top_Trigger_DirectStrike(TEXT("Top_Trigger_DirectStrike"));
		inline const FName Top_Trigger_ComboCashout(TEXT("Top_Trigger_ComboCashout"));
		inline const FName Top_Trigger_VulnerabilityShell(TEXT("Top_Trigger_VulnerabilityShell"));
		inline const FName Gate_Trigger_SpeedUp(TEXT("Gate_Trigger_SpeedUp"));
		inline const FName Gate_Trigger_RecoveryField(TEXT("Gate_Trigger_RecoveryField"));
		inline const FName Gate_Trigger_ReactiveRepair(TEXT("Gate_Trigger_ReactiveRepair"));
		inline const FName Gate_Trigger_ManaReactor(TEXT("Gate_Trigger_ManaReactor"));
		inline const FName Gate_Trigger_LaunchCharge(TEXT("Gate_Trigger_LaunchCharge"));
	}

	// RowNames for EffectTable
	namespace Effect
	{
		inline const FName Effect_ComboArc_01(TEXT("Effect_ComboArc_01"));
		inline const FName Effect_KineticShell_01(TEXT("Effect_KineticShell_01"));
		inline const FName Effect_PartyMana_02(TEXT("Effect_PartyMana_02"));
		inline const FName Effect_BossGroggy_02(TEXT("Effect_BossGroggy_02"));
		inline const FName Effect_ComboPulse_01(TEXT("Effect_ComboPulse_01"));
		inline const FName Effect_BloodOverdrive_01(TEXT("Effect_BloodOverdrive_01"));
		inline const FName Effect_GroggyHammer_02(TEXT("Effect_GroggyHammer_02"));
		inline const FName Effect_Shield_02(TEXT("Effect_Shield_02"));
		inline const FName Effect_RepairPickup_02(TEXT("Effect_RepairPickup_02"));
		inline const FName Effect_CounterShield_01(TEXT("Effect_CounterShield_01"));
		inline const FName Effect_StrengthCharge_02(TEXT("Effect_StrengthCharge_02"));
		inline const FName Effect_DirectStrike_02(TEXT("Effect_DirectStrike_02"));
		inline const FName Effect_ComboCashout_02(TEXT("Effect_ComboCashout_02"));
		inline const FName Effect_VulnerabilityShell_01(TEXT("Effect_VulnerabilityShell_01"));
		inline const FName Effect_RecoveryField_02(TEXT("Effect_RecoveryField_02"));
		inline const FName Effect_ReactiveRepair_01(TEXT("Effect_ReactiveRepair_01"));
		inline const FName Effect_ManaReactor_01(TEXT("Effect_ManaReactor_01"));
		inline const FName Effect_LaunchCharge_02(TEXT("Effect_LaunchCharge_02"));
		inline const FName Effect_VelocityBoost_01(TEXT("Effect_VelocityBoost_01"));
		inline const FName Effect_SpeedUp_01(TEXT("Effect_SpeedUp_01"));
	}

	/** 과거 장착 데이터에서만 읽는 식별자입니다. 새 데이터에는 사용하지 않습니다. */
	namespace LegacyBumper
	{
		inline const FName Rebound_CounterShell(TEXT("Rebound_CounterShell"));
		inline const FName Rebound_ManaOrb(TEXT("Rebound_ManaOrb"));
		inline const FName Side_LaunchCharge(TEXT("Side_LaunchCharge"));
		inline const FName Top_ComboUp(TEXT("Top_ComboUp"));
		inline const FName Top_ComboPickup(TEXT("Top_ComboPickup"));
		inline const FName Gate_ManaField(TEXT("Gate_ManaField"));
		inline const FName Gate_StrengthField(TEXT("Gate_StrengthField"));
	}

	inline FName NormalizeBumperRowId(const FName RowId)
	{
		if (RowId == LegacyBumper::Rebound_CounterShell)
		{
			return Bumper::Rebound_KineticShell;
		}
		if (RowId == LegacyBumper::Rebound_ManaOrb)
		{
			return Bumper::Rebound_BloodOverdrive;
		}
		if (RowId == LegacyBumper::Side_LaunchCharge)
		{
			return Bumper::Side_CounterShield;
		}
		if (RowId == LegacyBumper::Top_ComboUp)
		{
			return Bumper::Top_ComboArc;
		}
		if (RowId == LegacyBumper::Top_ComboPickup)
		{
			return Bumper::Top_VulnerabilityShell;
		}
		if (RowId == LegacyBumper::Gate_ManaField)
		{
			return Bumper::Gate_ReactiveRepair;
		}
		if (RowId == LegacyBumper::Gate_StrengthField)
		{
			return Bumper::Gate_ManaReactor;
		}
		return RowId;
	}
}
