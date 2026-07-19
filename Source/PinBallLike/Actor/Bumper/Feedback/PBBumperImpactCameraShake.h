// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "PBBumperImpactCameraShake.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBumperImpactCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UPBBumperImpactCameraShake(const FObjectInitializer& ObjectInitializer);
};
