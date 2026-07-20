// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"

namespace PBStatusEffectAssetIds
{
	namespace Type
	{
		inline const FPrimaryAssetType StatusEffectData(TEXT("StatusEffectData"));
	}

	namespace StatusEffect
	{
		inline const FName Burn(TEXT("Burn"));
		inline const FName Shield(TEXT("Shield"));
		inline const FName Strength(TEXT("Strength"));
		inline const FName BounceUp(TEXT("BounceUp"));
		inline const FName ZeroGravity(TEXT("ZeroGravity"));
		inline const FName InvincibleSkill(TEXT("InvincibleSkill"));
		inline const FName Piercing(TEXT("Piercing"));
	}
}
