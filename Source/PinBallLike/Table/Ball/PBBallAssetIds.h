// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"

namespace PBBallAssetIds
{
	namespace Type
	{
		inline const FPrimaryAssetType BallData(TEXT("BallData"));
	}

	namespace Ball
	{
		inline const FName Test01(TEXT("Test01"));
		inline const FName Test02(TEXT("Test02"));
	}

	namespace StarLevel
	{
		inline const FName Level_1100101(TEXT("Level_1100101"));
		inline const FName Level_1100102(TEXT("Level_1100102"));
		inline const FName Level_1100103(TEXT("Level_1100103"));
		inline const FName Level_1100201(TEXT("Level_1100201"));
		inline const FName Level_1100202(TEXT("Level_1100202"));
		inline const FName Level_1100203(TEXT("Level_1100203"));
	}

	
	namespace Skill
	{
		inline const FName Circular_Blade(TEXT("Circular_Blade"));
		inline const FName Slash(TEXT("Slash"));
		inline const FName Sword_Storm(TEXT("Sword_Storm"));
		inline const FName Charge_Laser(TEXT("Charge_Laser"));
		inline const FName Magic_Arrow(TEXT("Magic_Arrow"));
		inline const FName Revolver(TEXT("Revolver"));
		inline const FName Summon_Clone(TEXT("Summon_Clone"));
		inline const FName Summon_Bomb(TEXT("Summon_Bomb"));
		inline const FName Boomerang(TEXT("Boomerang"));
		inline const FName Saw_Blade(TEXT("Saw_Blade"));
		inline const FName Strength_Buff(TEXT("Strength_Buff"));
		inline const FName Speed_Buff(TEXT("Speed_Buff"));
		inline const FName Invincible_Buff(TEXT("Invincible_Buff"));
		inline const FName Bounce_Buff(TEXT("Bounce_Buff"));
		inline const FName ZeroGravity_Buff(TEXT("ZeroGravity_Buff"));
		inline const FName Weakness_Debuff(TEXT("Weakness_Debuff"));
	}
}
