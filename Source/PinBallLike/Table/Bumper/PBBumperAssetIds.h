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
		inline const FName Side_ShieldCharge(TEXT("Side_ShieldCharge"));
		inline const FName Side_ManaCharge(TEXT("Side_ManaCharge"));
		inline const FName Top_ComboUp(TEXT("Top_ComboUp"));
		inline const FName Top_GroggyStrike(TEXT("Top_GroggyStrike"));
		inline const FName Gate_SpeedUp(TEXT("Gate_SpeedUp"));
	}

	// RowNames for TriggerTable
	namespace Trigger
	{
		inline const FName Rebound_Trigger_PowerPush(TEXT("Rebound_Trigger_PowerPush"));
		inline const FName Rebound_Trigger_CounterShell(TEXT("Rebound_Trigger_CounterShell"));
		inline const FName Side_Trigger_ShieldCharge(TEXT("Side_Trigger_ShieldCharge"));
		inline const FName Side_Trigger_ManaCharge(TEXT("Side_Trigger_ManaCharge"));
		inline const FName Top_Trigger_ComboUp(TEXT("Top_Trigger_ComboUp"));
		inline const FName Top_Trigger_GroggyStrike(TEXT("Top_Trigger_GroggyStrike"));
		inline const FName Gate_Trigger_SpeedUp(TEXT("Gate_Trigger_SpeedUp"));
	}

	// RowNames for EffectTable
	namespace Effect
	{
		inline const FName Effect_ComboUp_01(TEXT("Effect_ComboUp_01"));
		inline const FName Effect_BossDamage_01(TEXT("Effect_BossDamage_01"));
		inline const FName Effect_PartyMana_01(TEXT("Effect_PartyMana_01"));
		inline const FName Effect_BossGroggy_01(TEXT("Effect_BossGroggy_01"));
	}
}
