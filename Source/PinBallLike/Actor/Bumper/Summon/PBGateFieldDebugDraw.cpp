#include "PBGateFieldDebugDraw.h"

#if ENABLE_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarPBGateFieldDebugMode(
		TEXT("pb.Bumper.DebugGateFields"),
		0,
		TEXT("Draws active Gate field bounds. 0=Off, 1=Top-down footprint, 2=Footprint and 3D collision sphere."),
		ECVF_Cheat);

	constexpr float DebugHeightOffset = 12.0f;
	constexpr float CenterMarkerRadius = 10.0f;
	constexpr float OriginMarkerRadius = 8.0f;
	constexpr float AxisHalfLength = 24.0f;
	constexpr float RingThickness = 5.0f;
}
#endif

void PBGateFieldDebugDraw::Draw(
	const UWorld* World,
	const FVector& FieldCenter,
	const float FieldRadius,
	const FColor& FieldColor,
	const TCHAR* FieldLabel,
	const bool bHasTriggerOrigin,
	const FVector& TriggerOrigin)
{
#if ENABLE_DRAW_DEBUG
	const int32 DebugMode = CVarPBGateFieldDebugMode.GetValueOnGameThread();
	if (DebugMode <= 0
		|| !IsValid(World)
		|| !FMath::IsFinite(FieldRadius)
		|| FieldRadius <= 0.0f
		|| FieldCenter.ContainsNaN())
	{
		return;
	}

	const FVector DrawCenter = FieldCenter + FVector(0.0f, 0.0f, DebugHeightOffset);
	const FColor SafeFieldColor = FieldColor == FColor::Black ? FColor::Cyan : FieldColor;

	DrawDebugCircle(
		World,
		DrawCenter,
		FieldRadius,
		64,
		SafeFieldColor,
		false,
		0.0f,
		0,
		RingThickness,
		FVector::ForwardVector,
		FVector::RightVector,
		false);

	DrawDebugSphere(
		World,
		DrawCenter,
		CenterMarkerRadius,
		12,
		FColor::White,
		false,
		0.0f,
		0,
		3.0f);
	DrawDebugLine(
		World,
		DrawCenter - FVector::ForwardVector * AxisHalfLength,
		DrawCenter + FVector::ForwardVector * AxisHalfLength,
		FColor::White,
		false,
		0.0f,
		0,
		2.0f);
	DrawDebugLine(
		World,
		DrawCenter - FVector::RightVector * AxisHalfLength,
		DrawCenter + FVector::RightVector * AxisHalfLength,
		FColor::White,
		false,
		0.0f,
		0,
		2.0f);

	float HorizontalOffset = 0.0f;
	if (bHasTriggerOrigin && !TriggerOrigin.ContainsNaN())
	{
		const FVector DrawTriggerOrigin = TriggerOrigin + FVector(0.0f, 0.0f, DebugHeightOffset);
		HorizontalOffset = (FieldCenter - TriggerOrigin).Size2D();
		DrawDebugSphere(
			World,
			DrawTriggerOrigin,
			OriginMarkerRadius,
			10,
			FColor::Yellow,
			false,
			0.0f,
			0,
			3.0f);
		DrawDebugLine(
			World,
			DrawTriggerOrigin,
			DrawCenter,
			FColor::Yellow,
			false,
			0.0f,
			0,
			3.0f);
		DrawDebugString(
			World,
			DrawTriggerOrigin + FVector(0.0f, 0.0f, 20.0f),
			TEXT("TRIGGER ORIGIN"),
			nullptr,
			FColor::Yellow,
			0.0f,
			true,
			0.9f);
	}

	DrawDebugString(
		World,
		DrawCenter + FVector(0.0f, 0.0f, 36.0f),
		FString::Printf(
			TEXT("%s | Radius=%.0f | Center=(%.0f, %.0f, %.0f) | XY Offset=%.0f"),
			FieldLabel ? FieldLabel : TEXT("GATE FIELD"),
			FieldRadius,
			FieldCenter.X,
			FieldCenter.Y,
			FieldCenter.Z,
			HorizontalOffset),
		nullptr,
		SafeFieldColor,
		0.0f,
		true,
		1.0f);

	if (DebugMode >= 2)
	{
		DrawDebugSphere(
			World,
			FieldCenter,
			FieldRadius,
			24,
			SafeFieldColor,
			false,
			0.0f,
			0,
			1.5f);
	}
#else
	static_cast<void>(World);
	static_cast<void>(FieldCenter);
	static_cast<void>(FieldRadius);
	static_cast<void>(FieldColor);
	static_cast<void>(FieldLabel);
	static_cast<void>(bHasTriggerOrigin);
	static_cast<void>(TriggerOrigin);
#endif
}
