// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBBossPatternData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBossPatternData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<UPBBossPatternBase> PatternClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	FName PatternName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float CooldownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsEnabled = true;
};
