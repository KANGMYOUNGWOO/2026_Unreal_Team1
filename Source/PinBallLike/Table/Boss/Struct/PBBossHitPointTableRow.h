// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Actor/Boss/Component/PBBossHitPartComponent.h"
#include "PBBossHitPointTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBossHitPointTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|HitPoint")
	FName BossRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|HitPoint")
	FName HitPointName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|HitPoint")
	EPBBossHitPartType HitPartType = EPBBossHitPartType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|HitPoint", meta = (ClampMin = "0"))
	int32 HPDamageMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|HitPoint", meta = (ClampMin = "0"))
	int32 GroggyMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|HitPoint")
	bool IsWeaknessPoint = false;
};
