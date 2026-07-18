#pragma once

#include "CoreMinimal.h"

/** Gate 영역의 배치, 판정, 표시가 함께 사용하는 공통 규칙입니다. */
namespace PBGateFieldTuning
{
	inline constexpr float DefaultRadius = 360.0f;
	inline constexpr float CollisionHalfHeight = 80.0f;
	inline constexpr float BasicShapeRadiusAtScaleOne = 50.0f;
	inline constexpr float MinimumRadius = 50.0f;
	inline constexpr float MaximumRadius = 800.0f;

	/** 영역 중심을 발동한 Gate Trigger 원점에 고정하고, Trigger 메시 스케일은 상속하지 않습니다. */
	inline FTransform MakeTransformAtGateOrigin(const FTransform& GateOriginTransform)
	{
		FTransform FieldTransform = GateOriginTransform;
		FieldTransform.SetScale3D(FVector::OneVector);
		return FieldTransform;
	}
}
