// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternPhaseType.generated.h"

UENUM(BlueprintType)
enum class EPBBossPatternPhaseType : uint8
{
	Normal,
	Enraged,
	EnragedEntry
};
