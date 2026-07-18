#pragma once

#include "CoreMinimal.h"

namespace PBGateFieldTuning
{
	inline constexpr float DefaultRadius = 360.0f;
	inline constexpr float CollisionHalfHeight = 80.0f;
	inline constexpr float BasicShapeRadiusAtScaleOne = 50.0f;
	inline constexpr float MinimumRadius = 50.0f;
	inline constexpr float MaximumRadius = 800.0f;

	inline FTransform MakeTransformAtGateOrigin(const FTransform& GateOriginTransform)
	{
		FTransform FieldTransform = GateOriginTransform;
		FieldTransform.SetScale3D(FVector::OneVector);
		return FieldTransform;
	}
}
