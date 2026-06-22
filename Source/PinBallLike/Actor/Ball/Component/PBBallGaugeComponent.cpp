// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallGaugeComponent.h"

#include "PinBallLike/Utils/PBFixedPoint.h"


UPBBallGaugeComponent::UPBBallGaugeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	FBallGaugeState HPGauge;
	HPGauge.CurrentRaw = FPBFixedPoint::ToRaw(100.0f);
	HPGauge.MaxRaw = FPBFixedPoint::ToRaw(100.0f);
	Gauges.Add(EBallGaugeType::EGT_HP, HPGauge);

	FBallGaugeState MPGauge;
	MPGauge.CurrentRaw = FPBFixedPoint::ToRaw(50.0f);
	MPGauge.MaxRaw = FPBFixedPoint::ToRaw(50.0f);
	MPGauge.RegenPerSecondRaw = FPBFixedPoint::ToRaw(1.0f);
	Gauges.Add(EBallGaugeType::EGT_MP, MPGauge);
}

void UPBBallGaugeComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TPair<EBallGaugeType, FBallGaugeState>& GaugePair : Gauges)
	{
		GaugePair.Value.MaxRaw = FMath::Max(0, GaugePair.Value.MaxRaw);
		GaugePair.Value.CurrentRaw = FPBFixedPoint::ClampRaw(GaugePair.Value.CurrentRaw, 0, GaugePair.Value.MaxRaw);
		GaugePair.Value.RegenPerSecondRaw = FMath::Max(0, GaugePair.Value.RegenPerSecondRaw);
	}
}

void UPBBallGaugeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
										  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<EBallGaugeType, FBallGaugeState>& GaugePair : Gauges)
	{
		FBallGaugeState& Gauge = GaugePair.Value;
		if (Gauge.RegenPerSecondRaw <= 0 || Gauge.CurrentRaw >= Gauge.MaxRaw)
		{
			continue;
		}

		const int32 PrevCurrent = Gauge.CurrentRaw;
		const double RegenRawDelta = static_cast<double>(Gauge.RegenPerSecondRaw) * DeltaTime + Gauge.RegenRawRemainder;
		const int32 RegenRawToApply = FMath::FloorToInt(RegenRawDelta);
		Gauge.RegenRawRemainder = RegenRawDelta - RegenRawToApply;

		Gauge.CurrentRaw = FPBFixedPoint::ClampRaw(Gauge.CurrentRaw + RegenRawToApply, 0, Gauge.MaxRaw);
		if (Gauge.CurrentRaw >= Gauge.MaxRaw)
		{
			Gauge.RegenRawRemainder = 0.0;
		}

		if (PrevCurrent != Gauge.CurrentRaw)
		{
			BroadcastGaugeChanged(GaugePair.Key, Gauge);
		}
	}
}

bool UPBBallGaugeComponent::HasGauge(EBallGaugeType Type) const
{
	return Gauges.Contains(Type);
}

float UPBBallGaugeComponent::GetCurrent(EBallGaugeType Type) const
{
	const FBallGaugeState* Gauge = FindGauge(Type);
	return Gauge ? FPBFixedPoint::ToFloat(Gauge->CurrentRaw) : 0.0f;
}

float UPBBallGaugeComponent::GetMax(EBallGaugeType Type) const
{
	const FBallGaugeState* Gauge = FindGauge(Type);
	return Gauge ? FPBFixedPoint::ToFloat(Gauge->MaxRaw) : 0.0f;
}

float UPBBallGaugeComponent::GetRatio(EBallGaugeType Type) const
{
	const FBallGaugeState* Gauge = FindGauge(Type);
	return Gauge && Gauge->MaxRaw > 0 ? static_cast<float>(Gauge->CurrentRaw) / Gauge->MaxRaw : 0.0f;
}

void UPBBallGaugeComponent::SetGauge(EBallGaugeType Type, float Current, float Max)
{
	FBallGaugeState& Gauge = Gauges.FindOrAdd(Type);
	Gauge.MaxRaw = FPBFixedPoint::ToRawNonNegative(Max);
	Gauge.CurrentRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(Current), 0, Gauge.MaxRaw);
	Gauge.RegenRawRemainder = 0.0;
	BroadcastGaugeChanged(Type, Gauge);
}

void UPBBallGaugeComponent::SetCurrent(EBallGaugeType Type, float Value)
{
	FBallGaugeState* Gauge = FindGauge(Type);
	if (!Gauge)
	{
		return;
	}

	Gauge->CurrentRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(Value), 0, Gauge->MaxRaw);
	if (Gauge->CurrentRaw >= Gauge->MaxRaw)
	{
		Gauge->RegenRawRemainder = 0.0;
	}
	BroadcastGaugeChanged(Type, *Gauge);
}

void UPBBallGaugeComponent::SetMax(EBallGaugeType Type, float Value, bool bFillCurrent)
{
	FBallGaugeState* Gauge = FindGauge(Type);
	if (!Gauge)
	{
		return;
	}

	Gauge->MaxRaw = FPBFixedPoint::ToRawNonNegative(Value);
	Gauge->CurrentRaw = bFillCurrent ? Gauge->MaxRaw : FPBFixedPoint::ClampRaw(Gauge->CurrentRaw, 0, Gauge->MaxRaw);
	if (Gauge->CurrentRaw >= Gauge->MaxRaw)
	{
		Gauge->RegenRawRemainder = 0.0;
	}
	BroadcastGaugeChanged(Type, *Gauge);
}

void UPBBallGaugeComponent::SetRegenPerSecond(EBallGaugeType Type, float Value)
{
	FBallGaugeState* Gauge = FindGauge(Type);
	if (!Gauge)
	{
		return;
	}

	Gauge->RegenPerSecondRaw = FPBFixedPoint::ToRawNonNegative(Value);
	Gauge->RegenRawRemainder = 0.0;
}

void UPBBallGaugeComponent::ApplyDelta(EBallGaugeType Type, float Delta)
{
	FBallGaugeState* Gauge = FindGauge(Type);
	const int32 RawDelta = FPBFixedPoint::ToRaw(Delta);
	if (!Gauge || FPBFixedPoint::IsNearlyZeroRaw(RawDelta))
	{
		return;
	}

	Gauge->CurrentRaw = FPBFixedPoint::ClampRaw(Gauge->CurrentRaw + RawDelta, 0, Gauge->MaxRaw);
	if (Gauge->CurrentRaw >= Gauge->MaxRaw)
	{
		Gauge->RegenRawRemainder = 0.0;
	}
	BroadcastGaugeChanged(Type, *Gauge);
}

bool UPBBallGaugeComponent::CanConsume(EBallGaugeType Type, float Cost) const
{
	const FBallGaugeState* Gauge = FindGauge(Type);
	const int32 RawCost = FPBFixedPoint::ToRaw(Cost);
	return Gauge && Cost >= 0.0f && RawCost >= 0 && Gauge->CurrentRaw >= RawCost;
}

bool UPBBallGaugeComponent::Consume(EBallGaugeType Type, float Cost)
{
	if (!CanConsume(Type, Cost))
	{
		return false;
	}

	ApplyDelta(Type, -Cost);
	return true;
}

FBallGaugeState* UPBBallGaugeComponent::FindGauge(EBallGaugeType Type)
{
	return Gauges.Find(Type);
}

const FBallGaugeState* UPBBallGaugeComponent::FindGauge(EBallGaugeType Type) const
{
	return Gauges.Find(Type);
}

void UPBBallGaugeComponent::BroadcastGaugeChanged(EBallGaugeType Type, const FBallGaugeState& Gauge)
{
	OnGaugeChanged.Broadcast(Type, FPBFixedPoint::ToFloat(Gauge.CurrentRaw), FPBFixedPoint::ToFloat(Gauge.MaxRaw));
}
