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
enum class EPBBumperRoleType : uint8
{
	Attack,
	Spawn,
	Support,
	Zone
};

UENUM(BlueprintType)
enum class EPBBumperEffectType : uint8
{
	Instant,
	Buff,
	Area,
	Summon
};

UENUM(BlueprintType)
enum class EPBBumperEffectExecutionPolicy : uint8
{
	Immediate,
	QueueIfBusy
};
