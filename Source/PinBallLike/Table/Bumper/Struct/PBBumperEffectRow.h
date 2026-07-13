// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperEffectRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	EPBBumperEffectType EffectType = EPBBumperEffectType::Instant;

	/** 효과의 완료 계약. Immediate도 잘못 장기 실행되면 공유 Effect 재진입을 막기 위해 안전하게 대기한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	EPBBumperEffectExecutionPolicy ExecutionPolicy = EPBBumperEffectExecutionPolicy::Immediate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	float Power = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FText Description;

};
