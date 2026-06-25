// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBaseResourceComponent.h"

#include "PBResourceTypes.h"
#include "PinBallLike/Utils/PBFixedPoint.h"

UPBBaseResourceComponent::UPBBaseResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UPBBaseResourceComponent::HasResource(FName ResourceName) const
{
	return Resources.Contains(ResourceName);
}

float UPBBaseResourceComponent::GetResourceCurrent(FName ResourceName) const
{
	return GetCurrent(ResourceName);
}

float UPBBaseResourceComponent::GetResourceMax(FName ResourceName) const
{
	return GetMax(ResourceName);
}

float UPBBaseResourceComponent::GetResourceRatio(FName ResourceName) const
{
	return GetRatio(ResourceName);
}

void UPBBaseResourceComponent::TakeDamage(int32 Damage)
{
	if (Damage <= 0)
	{
		return;
	}

	ApplyResourceDelta(PBResourceNames::Health, -static_cast<float>(Damage));
}

bool UPBBaseResourceComponent::IsDead() const
{
	return GetCurrent(PBResourceNames::Health) <= 0.0f;
}

void UPBBaseResourceComponent::SetResource(FName ResourceName, float Current, float Max)
{
	FPBResourceState& Resource = Resources.FindOrAdd(ResourceName);
	const int32 PrevCurrentRaw = Resource.CurrentRaw;
	Resource.MaxRaw = FPBFixedPoint::ToRawNonNegative(Max);
	Resource.CurrentRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(Current), 0, Resource.MaxRaw);
	Resource.RegenRawRemainder = 0.0;
	OnResourceChanged.Broadcast(ResourceName);
	if (PrevCurrentRaw != Resource.CurrentRaw)
	{
		OnResourceCurrentChanged.Broadcast(ResourceName, FPBFixedPoint::ToFloat(Resource.CurrentRaw));
	}
}

void UPBBaseResourceComponent::SetResourceCurrent(FName ResourceName, float Value)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	if (!Resource)
	{
		return;
	}

	const int32 PrevCurrentRaw = Resource->CurrentRaw;
	Resource->CurrentRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(Value), 0, Resource->MaxRaw);
	if (PrevCurrentRaw == Resource->CurrentRaw)
	{
		return;
	}

	if (Resource->CurrentRaw >= Resource->MaxRaw)
	{
		Resource->RegenRawRemainder = 0.0;
	}

	OnResourceCurrentChanged.Broadcast(ResourceName, FPBFixedPoint::ToFloat(Resource->CurrentRaw));
}

void UPBBaseResourceComponent::SetResourceMax(FName ResourceName, float Value, bool bFillCurrent)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	if (!Resource)
	{
		return;
	}

	const int32 PrevMaxRaw = Resource->MaxRaw;
	const int32 PrevCurrentRaw = Resource->CurrentRaw;
	Resource->MaxRaw = FPBFixedPoint::ToRawNonNegative(Value);
	Resource->CurrentRaw = bFillCurrent ? Resource->MaxRaw : FPBFixedPoint::ClampRaw(Resource->CurrentRaw, 0, Resource->MaxRaw);
	if (PrevMaxRaw == Resource->MaxRaw && PrevCurrentRaw == Resource->CurrentRaw)
	{
		return;
	}

	if (Resource->CurrentRaw >= Resource->MaxRaw)
	{
		Resource->RegenRawRemainder = 0.0;
	}
	OnResourceChanged.Broadcast(ResourceName);
	if (PrevCurrentRaw != Resource->CurrentRaw)
	{
		OnResourceCurrentChanged.Broadcast(ResourceName, FPBFixedPoint::ToFloat(Resource->CurrentRaw));
	}
}

void UPBBaseResourceComponent::SetResourceRegenPerSecond(FName ResourceName, float Value)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	if (!Resource)
	{
		return;
	}

	const int32 PrevRegenPerSecondRaw = Resource->RegenPerSecondRaw;
	Resource->RegenPerSecondRaw = FPBFixedPoint::ToRawNonNegative(Value);
	if (PrevRegenPerSecondRaw == Resource->RegenPerSecondRaw)
	{
		return;
	}

	Resource->RegenRawRemainder = 0.0;
	OnResourceChanged.Broadcast(ResourceName);
}

void UPBBaseResourceComponent::ApplyResourceDelta(FName ResourceName, float Delta)
{
	FPBResourceState* Resource = FindResource(ResourceName);
	const int32 RawDelta = FPBFixedPoint::ToRaw(Delta);
	if (!Resource || FPBFixedPoint::IsNearlyZeroRaw(RawDelta))
	{
		return;
	}

	const int32 PrevCurrentRaw = Resource->CurrentRaw;
	Resource->CurrentRaw = FPBFixedPoint::ClampRaw(Resource->CurrentRaw + RawDelta, 0, Resource->MaxRaw);
	if (PrevCurrentRaw == Resource->CurrentRaw)
	{
		return;
	}

	if (Resource->CurrentRaw >= Resource->MaxRaw)
	{
		Resource->RegenRawRemainder = 0.0;
	}
	OnResourceCurrentChanged.Broadcast(ResourceName, FPBFixedPoint::ToFloat(Resource->CurrentRaw));
}

bool UPBBaseResourceComponent::CanConsumeResource(FName ResourceName, float Cost) const
{
	const FPBResourceState* Resource = FindResource(ResourceName);
	const int32 RawCost = FPBFixedPoint::ToRaw(Cost);
	return Resource && Cost >= 0.0f && RawCost >= 0 && Resource->CurrentRaw >= RawCost;
}

bool UPBBaseResourceComponent::ConsumeResource(FName ResourceName, float Cost)
{
	if (!CanConsumeResource(ResourceName, Cost))
	{
		return false;
	}

	ApplyResourceDelta(ResourceName, -Cost);
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
			OnResourceCurrentChanged.Broadcast(ResourcePair.Key, FPBFixedPoint::ToFloat(Resource.CurrentRaw));
		}
	}
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

FPBResourceState* UPBBaseResourceComponent::FindResource(FName ResourceName)
{
	return Resources.Find(ResourceName);
}

const FPBResourceState* UPBBaseResourceComponent::FindResource(FName ResourceName) const
{
	return Resources.Find(ResourceName);
}