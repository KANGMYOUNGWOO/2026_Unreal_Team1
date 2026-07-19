// Copyright Epic Games, Inc. All Rights Reserved.

#include "PBBumperImpactCameraShake.h"

#include "Shakes/PerlinNoiseCameraShakePattern.h"

UPBBumperImpactCameraShake::UPBBumperImpactCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPerlinNoiseCameraShakePattern>(TEXT("RootShakePattern")))
{
	bSingleInstance = true;

	UPerlinNoiseCameraShakePattern* Pattern = CastChecked<UPerlinNoiseCameraShakePattern>(GetRootShakePattern());
	Pattern->Duration = 0.18f;
	Pattern->BlendInTime = 0.01f;
	Pattern->BlendOutTime = 0.10f;

	Pattern->X.Amplitude = 4.0f;
	Pattern->X.Frequency = 20.0f;
	Pattern->Y.Amplitude = 4.0f;
	Pattern->Y.Frequency = 23.0f;
	Pattern->Z.Amplitude = 0.8f;
	Pattern->Z.Frequency = 18.0f;

	Pattern->Pitch.Amplitude = 0.18f;
	Pattern->Pitch.Frequency = 20.0f;
	Pattern->Yaw.Amplitude = 0.18f;
	Pattern->Yaw.Frequency = 22.0f;
	Pattern->Roll.Amplitude = 0.26f;
	Pattern->Roll.Frequency = 20.0f;

	Pattern->FOV.Amplitude = 0.0f;
	Pattern->FOV.Frequency = 0.0f;
}
