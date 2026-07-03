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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	float Power = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FText Description;

};
