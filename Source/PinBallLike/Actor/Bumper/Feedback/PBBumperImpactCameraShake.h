// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "PBBumperImpactCameraShake.generated.h"

/**
 * 범퍼의 기본 물리 반발에 사용하는 짧고 약한 카메라 셰이크다.
 * 단일 인스턴스로 동작해 빠른 연속 충돌에서도 셰이크가 중첩되지 않는다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBumperImpactCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UPBBumperImpactCameraShake(const FObjectInitializer& ObjectInitializer);
};
