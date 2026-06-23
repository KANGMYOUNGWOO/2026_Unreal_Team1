// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBBumperTypes.generated.h"

UENUM(BlueprintType)
enum class EPBBumperType : uint8
{
	Rebound,
	Side,
	TopTarget,
	Gate
};

UENUM(BlueprintType)
enum class EPBBumperTriggerType : uint8
{
	HitCount,
	PassCount,
	RailEnter
};

UENUM(BlueprintType)
enum class EPBBumperActivationType : uint8
{
	Attack,
	Spawn,
	Support,
	Area
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	FName BumperName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	EPBBumperType BumperType = EPBBumperType::Rebound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	EPBBumperTriggerType TriggerType = EPBBumperTriggerType::HitCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper")
	EPBBumperActivationType ActivationType = EPBBumperActivationType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper", meta = (ClampMin = "1"))
	int32 RequiredTriggerCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper", meta = (ClampMin = "1", ClampMax = "3"))
	int32 StarGrade = 1;
};
