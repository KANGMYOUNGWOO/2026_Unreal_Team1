// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperTableRow.generated.h"

class UPBBumperDataAsset;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	EPBBumperType BumperType = EPBBumperType::Rebound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	FName TriggerID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	EPBBumperRoleType RoleType = EPBBumperRoleType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	EPBBumperEffectType EffectType = EPBBumperEffectType::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table", meta = (ClampMin = "1"))
	int32 RequiredTriggerCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	FName EffectID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Table")
	TSoftObjectPtr<UPBBumperDataAsset> BumperDataAsset;
};
