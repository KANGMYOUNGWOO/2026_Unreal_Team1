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

	/** 실제 Trigger 진행도를 증가시킨 유효 접촉의 전투 누적 횟수입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime|Telemetry")
	int32 MeaningfulContactCount = 0;

	/** 대기 요청이 아니라 효과 실행 레인에 실제 진입한 전투 누적 횟수입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime|Telemetry")
	int32 ActivationCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Runtime")
	EPBBumperState CurrentState = EPBBumperState::Idle;
};
