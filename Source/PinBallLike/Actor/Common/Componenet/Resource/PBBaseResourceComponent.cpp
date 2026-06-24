// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBaseResourceComponent.h"

#include "PinBallLike/Utils/PBFixedPoint.h"

UPBBaseResourceComponent::UPBBaseResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UPBBaseResourceComponent::HasResource(FName ResourceName) const
{
	return Resources.Contains(ResourceName);
}

float UPBBaseResourceComponent::GetCurrent(FName ResourceName) const
{
	const FPBResourceState* Resource = FindResource(ResourceName);
	return Resource ? FPBFixedPoint::ToFloat(Resource->CurrentRaw) : 0.0f;
}

float UPBBaseResourceComponent::GetMax(FName ResourceName) const
{
	const FPBResourceState* Resource = FindResource(ResourceName);
	return Resource ? FPBFixedPoint::ToFloat(Resource->MaxRaw) : 0.0f;
}

float UPBBaseResourceComponent::GetRatio(FName ResourceName) const
{
	const FPBResourceState* Resource = FindResource(ResourceName);
	return Resource && Resource->MaxRaw > 0 ? static_cast<float>(Resource->CurrentRaw) / Resource->MaxRaw : 0.0f;
}

void UPBBaseResourceComponent::SetResource(FName ResourceName, float Current, float Max)
{
	FPBResourceState& Resource = Resources.FindOrAdd(ResourceName);
	Resource.MaxRaw = FPBFixedPoint::ToRawNonNegative(Max);
	Resource.CurrentRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(Current), 0, Resource.MaxRaw);
	Resource.RegenRawRemainder = 0.0;
	BroadcastResourceChanged(ResourceName, Resource);
}

void UPBBaseResourceComponent::SetCurrent(FName ResourceName, float Value)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	if (!Resource)
	{
		return;
	}

	Resource->CurrentRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(Value), 0, Resource->MaxRaw);
	if (Resource->CurrentRaw >= Resource->MaxRaw)
	{
		Resource->RegenRawRemainder = 0.0;
	}
	BroadcastResourceChanged(ResourceName, *Resource);
}

void UPBBaseResourceComponent::SetMax(FName ResourceName, float Value, bool bFillCurrent)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	if (!Resource)
	{
		return;
	}

	Resource->MaxRaw = FPBFixedPoint::ToRawNonNegative(Value);
	Resource->CurrentRaw = bFillCurrent ? Resource->MaxRaw : FPBFixedPoint::ClampRaw(Resource->CurrentRaw, 0, Resource->MaxRaw);
	if (Resource->CurrentRaw >= Resource->MaxRaw)
	{
		Resource->RegenRawRemainder = 0.0;
	}
	BroadcastResourceChanged(ResourceName, *Resource);
}

void UPBBaseResourceComponent::SetRegenPerSecond(FName ResourceName, float Value)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	if (!Resource)
	{
		return;
	}

	Resource->RegenPerSecondRaw = FPBFixedPoint::ToRawNonNegative(Value);
	Resource->RegenRawRemainder = 0.0;
}

void UPBBaseResourceComponent::ApplyDelta(FName ResourceName, float Delta)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	const int32 RawDelta = FPBFixedPoint::ToRaw(Delta);
	if (!Resource || FPBFixedPoint::IsNearlyZeroRaw(RawDelta))
	{
		return;
	}

	Resource->CurrentRaw = FPBFixedPoint::ClampRaw(Resource->CurrentRaw + RawDelta, 0, Resource->MaxRaw);
	if (Resource->CurrentRaw >= Resource->MaxRaw)
	{
		Resource->RegenRawRemainder = 0.0;
	}
	BroadcastResourceChanged(ResourceName, *Resource);
}

bool UPBBaseResourceComponent::CanConsume(FName ResourceName, float Cost) const
{
	const FPBResourceState* Resource = FindResource(ResourceName);
	const int32 RawCost = FPBFixedPoint::ToRaw(Cost);
	return Resource && Cost >= 0.0f && RawCost >= 0 && Resource->CurrentRaw >= RawCost;
}

bool UPBBaseResourceComponent::Consume(FName ResourceName, float Cost)
{
	if (!CanConsume(ResourceName, Cost))
	{
		return false;
	}

	ApplyDelta(ResourceName, -Cost);
	return true;
}

void UPBBaseResourceComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TPair<FName, FPBResourceState>& ResourcePair : Resources)
	{
		FPBResourceState& Resource = ResourcePair.Value;
		Resource.MaxRaw = FMath::Max(0, Resource.MaxRaw);
		Resource.CurrentRaw = FPBFixedPoint::ClampRaw(Resource.CurrentRaw, 0, Resource.MaxRaw);
		Resource.RegenPerSecondRaw = FMath::Max(0, Resource.RegenPerSecondRaw);
	}
}

void UPBBaseResourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<FName, FPBResourceState>& ResourcePair : Resources)
	{
		FPBResourceState& Resource = ResourcePair.Value;
		if (Resource.RegenPerSecondRaw <= 0 || Resource.CurrentRaw >= Resource.MaxRaw)
		{
			continue;
		}

		const int32 PrevCurrent = Resource.CurrentRaw;
		const double RegenRawDelta = static_cast<double>(Resource.RegenPerSecondRaw) * DeltaTime + Resource.RegenRawRemainder;
		const int32 RegenRawToApply = FMath::FloorToInt(RegenRawDelta);
		Resource.RegenRawRemainder = RegenRawDelta - RegenRawToApply;

		Resource.CurrentRaw = FPBFixedPoint::ClampRaw(Resource.CurrentRaw + RegenRawToApply, 0, Resource.MaxRaw);
		if (Resource.CurrentRaw >= Resource.MaxRaw)
		{
			Resource.RegenRawRemainder = 0.0;
		}

		if (PrevCurrent != Resource.CurrentRaw)
		{
			BroadcastResourceChanged(ResourcePair.Key, Resource);
		}
	}
}

FPBResourceState* UPBBaseResourceComponent::FindResource(FName ResourceName)
{
	return Resources.Find(ResourceName);
}

const FPBResourceState* UPBBaseResourceComponent::FindResource(FName ResourceName) const
{
	return Resources.Find(ResourceName);
}

void UPBBaseResourceComponent::BroadcastResourceChanged(FName ResourceName, const FPBResourceState& Resource)
{
	(void)ResourceName;
	(void)Resource;
}
