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
		inline const FName BasicCombo(TEXT("BasicCombo"));
		inline const FName Test01(TEXT("Test01"));
		inline const FName Test02(TEXT("Test02"));
		inline const FName Test03(TEXT("Test03"));
		inline const FName Gate01(TEXT("Gate01"));
	}

	// RowNames forTriggerTable
	namespace Trigger
	{
		inline const FName Circle_Trigger_1(TEXT("Circle_Trigger_1"));
		inline const FName Trigger_Test01(TEXT("Trigger_Test01"));
		inline const FName Trigger_Test02(TEXT("Trigger_Test02"));
		inline const FName Trigger_Test03(TEXT("Trigger_Test03"));
		inline const FName Rebound_Trigger(TEXT("Rebound_Trigger"));
		inline const FName GateBumper(TEXT("GateBumper"));
	}

	// RowNames for EffectTable
	namespace Effect
	{
		inline const FName Effect_ComboUp_01(TEXT("Effect_ComboUp_01"));
	}
}
