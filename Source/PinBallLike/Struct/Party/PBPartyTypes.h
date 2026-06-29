// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBPartyTypes.generated.h"

UENUM(BlueprintType)
enum class EPBBallPartyRole : uint8
{
	None,
	Leader,
	Follower
};
