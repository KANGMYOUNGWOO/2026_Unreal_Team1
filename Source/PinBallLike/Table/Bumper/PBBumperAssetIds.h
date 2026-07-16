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
		inline const FName Rebound_CounterShell(TEXT("Rebound_CounterShell"));
		inline const FName Rebound_ComboPulse(TEXT("Rebound_ComboPulse"));
		inline const FName Rebound_ManaOrb(TEXT("Rebound_ManaOrb"));
		inline const FName Rebound_GroggyHammer(TEXT("Rebound_GroggyHammer"));
		inline const FName Side_ShieldCharge(TEXT("Side_ShieldCharge"));
		inline const FName Side_ManaCharge(TEXT("Side_ManaCharge"));
		inline const FName Side_RepairPickup(TEXT("Side_RepairPickup"));
		inline const FName Side_LaunchCharge(TEXT("Side_LaunchCharge"));
		inline const FName Side_StrengthCharge(TEXT("Side_StrengthCharge"));
		inline const FName Top_ComboUp(TEXT("Top_ComboUp"));
		inline const FName Top_GroggyStrike(TEXT("Top_GroggyStrike"));
		inline const FName Top_DirectStrike(TEXT("Top_DirectStrike"));
		inline const FName Top_ComboCashout(TEXT("Top_ComboCashout"));
		inline const FName Top_ComboPickup(TEXT("Top_ComboPickup"));
		inline const FName Gate_SpeedUp(TEXT("Gate_SpeedUp"));
		inline const FName Gate_RecoveryField(TEXT("Gate_RecoveryField"));
		inline const FName Gate_ManaField(TEXT("Gate_ManaField"));
		inline const FName Gate_StrengthField(TEXT("Gate_StrengthField"));
		inline const FName Gate_LaunchCharge(TEXT("Gate_LaunchCharge"));
	}

	// RowNames for TriggerTable
	namespace Trigger
	{
		inline const FName Rebound_Trigger_PowerPush(TEXT("Rebound_Trigger_PowerPush"));
		inline const FName Rebound_Trigger_CounterShell(TEXT("Rebound_Trigger_CounterShell"));
		inline const FName Rebound_Trigger_ComboPulse(TEXT("Rebound_Trigger_ComboPulse"));
		inline const FName Rebound_Trigger_ManaOrb(TEXT("Rebound_Trigger_ManaOrb"));
		inline const FName Rebound_Trigger_GroggyHammer(TEXT("Rebound_Trigger_GroggyHammer"));
		inline const FName Side_Trigger_ShieldCharge(TEXT("Side_Trigger_ShieldCharge"));
		inline const FName Side_Trigger_ManaCharge(TEXT("Side_Trigger_ManaCharge"));
		inline const FName Side_Trigger_RepairPickup(TEXT("Side_Trigger_RepairPickup"));
		inline const FName Side_Trigger_LaunchCharge(TEXT("Side_Trigger_LaunchCharge"));
		inline const FName Side_Trigger_StrengthCharge(TEXT("Side_Trigger_StrengthCharge"));
		inline const FName Top_Trigger_ComboUp(TEXT("Top_Trigger_ComboUp"));
		inline const FName Top_Trigger_GroggyStrike(TEXT("Top_Trigger_GroggyStrike"));
		inline const FName Top_Trigger_DirectStrike(TEXT("Top_Trigger_DirectStrike"));
		inline const FName Top_Trigger_ComboCashout(TEXT("Top_Trigger_ComboCashout"));
		inline const FName Top_Trigger_ComboPickup(TEXT("Top_Trigger_ComboPickup"));
		inline const FName Gate_Trigger_SpeedUp(TEXT("Gate_Trigger_SpeedUp"));
		inline const FName Gate_Trigger_RecoveryField(TEXT("Gate_Trigger_RecoveryField"));
		inline const FName Gate_Trigger_ManaField(TEXT("Gate_Trigger_ManaField"));
		inline const FName Gate_Trigger_StrengthField(TEXT("Gate_Trigger_StrengthField"));
		inline const FName Gate_Trigger_LaunchCharge(TEXT("Gate_Trigger_LaunchCharge"));
	}

	// RowNames for EffectTable
	namespace Effect
	{
		inline const FName Effect_ComboUp_01(TEXT("Effect_ComboUp_01"));
		inline const FName Effect_BossDamage_01(TEXT("Effect_BossDamage_01"));
		inline const FName Effect_PartyMana_01(TEXT("Effect_PartyMana_01"));
		inline const FName Effect_BossGroggy_01(TEXT("Effect_BossGroggy_01"));
		inline const FName Effect_ComboPulse_01(TEXT("Effect_ComboPulse_01"));
		inline const FName Effect_ManaPickup_01(TEXT("Effect_ManaPickup_01"));
		inline const FName Effect_GroggyHammer_01(TEXT("Effect_GroggyHammer_01"));
		inline const FName Effect_RepairPickup_01(TEXT("Effect_RepairPickup_01"));
		inline const FName Effect_LaunchCharge_01(TEXT("Effect_LaunchCharge_01"));
		inline const FName Effect_StrengthCharge_01(TEXT("Effect_StrengthCharge_01"));
		inline const FName Effect_DirectStrike_01(TEXT("Effect_DirectStrike_01"));
		inline const FName Effect_ComboCashout_01(TEXT("Effect_ComboCashout_01"));
		inline const FName Effect_ComboPickup_01(TEXT("Effect_ComboPickup_01"));
		inline const FName Effect_RecoveryField_01(TEXT("Effect_RecoveryField_01"));
		inline const FName Effect_ManaField_01(TEXT("Effect_ManaField_01"));
		inline const FName Effect_StrengthField_01(TEXT("Effect_StrengthField_01"));
	}
}
