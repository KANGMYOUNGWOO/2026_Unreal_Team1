// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBBossTableRow.generated.h"

class UPBBossDataAsset;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBossTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table", meta = (ClampMin = "1"))
	int32 MaxHP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table", meta = (ClampMin = "1"))
	int32 MaxGroggyGauge = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table", meta = (ClampMin = "0.1"))
	float GroggyDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table", meta = (ClampMin = "0", ClampMax = "100"))
	int32 EnrageHPRatioPercent = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table")
	FName DefaultHitPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table", meta = (ClampMin = "0"))
	float DamageCooldownSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table", meta = (ClampMin = "0.1"))
	float PatternCheckIntervalSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Table")
	TSoftObjectPtr<UPBBossDataAsset> BossDataAsset;
};
