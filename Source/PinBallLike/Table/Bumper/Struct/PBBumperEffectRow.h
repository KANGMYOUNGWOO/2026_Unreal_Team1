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
	EPBBumperEffectExecutionPolicy ExecutionPolicy = EPBBumperEffectExecutionPolicy::Immediate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	float Power = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	float SecondaryPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect", meta = (ClampMin = "0"))
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName SharedEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName ActivationVfxId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName DeliveryVfxId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName ImpactVfxId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName StatusVfxId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FText Description;

};
