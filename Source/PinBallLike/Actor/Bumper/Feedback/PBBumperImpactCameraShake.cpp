// Copyright Epic Games, Inc. All Rights Reserved.

#include "PBBumperImpactCameraShake.h"

#include "Shakes/PerlinNoiseCameraShakePattern.h"

UPBBumperImpactCameraShake::UPBBumperImpactCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPerlinNoiseCameraShakePattern>(TEXT("RootShakePattern")))
{
	bSingleInstance = true;

	UPerlinNoiseCameraShakePattern* Pattern = CastChecked<UPerlinNoiseCameraShakePattern>(GetRootShakePattern());
	Pattern->Duration = 0.14f;
	Pattern->BlendInTime = 0.01f;
	Pattern->BlendOutTime = 0.08f;

	Pattern->X.Amplitude = 1.2f;
	Pattern->X.Frequency = 24.0f;
	Pattern->Y.Amplitude = 1.2f;
	Pattern->Y.Frequency = 27.0f;
	Pattern->Z.Amplitude = 0.35f;
	Pattern->Z.Frequency = 20.0f;

	Pattern->Pitch.Amplitude = 0.08f;
	Pattern->Pitch.Frequency = 22.0f;
	Pattern->Yaw.Amplitude = 0.08f;
	Pattern->Yaw.Frequency = 25.0f;
	Pattern->Roll.Amplitude = 0.12f;
	Pattern->Roll.Frequency = 24.0f;

	// 짧은 충돌 피드백에서 화면 확대까지 흔들리면 답답할 수 있어 FOV 변화는 사용하지 않는다.
	Pattern->FOV.Amplitude = 0.0f;
	Pattern->FOV.Frequency = 0.0f;
}
