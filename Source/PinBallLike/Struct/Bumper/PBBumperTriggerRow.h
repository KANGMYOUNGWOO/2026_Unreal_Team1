// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperTriggerRow.generated.h"

class UPBBumperTriggerDataAsset;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperTriggerRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Trigger")
	EPBBumperTriggerType TriggerType = EPBBumperTriggerType::HitCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Trigger")
	TArray<EPBBumperPositionId> PositionIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Trigger")
	FText TriggerDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Trigger")
	TSoftObjectPtr<UPBBumperTriggerDataAsset> TriggerDataAsset;
};
