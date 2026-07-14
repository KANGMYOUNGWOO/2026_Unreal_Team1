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
		inline const FName Charge_Laser(TEXT("Charge_Laser"));
		inline const FName Buff_Test(TEXT("Buff_Test"));
	}
}
