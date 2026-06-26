// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBBumperTriggerSpawnInfo.generated.h"

class APBBumperTriggerActorBase;

UENUM(BlueprintType)
enum class EPBBumperPositionId : uint8
{
	None,
	ReboundLeft,
	ReboundRight,

	SideLeft,
	SideRight,

	GateCenterMid,
	GateCenterLeft,
	GateCenterRight,

	TopTargetLeft,
	TopTargetRight,
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperTriggerSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TSubclassOf<APBBumperTriggerActorBase> TriggerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger")
	TArray<EPBBumperPositionId> PositionIds;
};
