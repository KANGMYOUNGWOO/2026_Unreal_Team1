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
		inline const FPrimaryAssetType TriggerData(TEXT("BumperTriggerData"));
		inline const FPrimaryAssetType EffectData(TEXT("BumperEffectData"));
	}

	// RowNames for BumperTable
	namespace Bumper
	{
		inline const FName BasicCombo(TEXT("BasicCombo"));
		inline const FName BasicTest02(TEXT("BasicTest02"));
		inline const FName BasicTest03(TEXT("BasicTest03"));
	}

	// RowNames forTriggerTable
	namespace Trigger
	{
		inline const FName Circle_Trigger_1(TEXT("Circle_Trigger_1"));
		inline const FName Trigger_Hit_2(TEXT("Trigger_Hit_2"));
	}

	// RowNames for EffectTable
	namespace Effect
	{
		inline const FName Effect_ComboUp_01(TEXT("Effect_ComboUp_01"));
	}
}
