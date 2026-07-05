// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperState.h"
#include "PBBumperRuntimeState.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime")
	int32 CurrentTriggerCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime")
	EPBBumperState CurrentState = EPBBumperState::Idle;
};
