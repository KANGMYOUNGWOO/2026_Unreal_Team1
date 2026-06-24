// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBaseStatComponent.h"

UPBBaseStatComponent::UPBBaseStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBaseStatComponent::HasStat(FName StatName) const
{
	return Stats.Contains(StatName);
}

int32 UPBBaseStatComponent::GetStat(FName StatName) const
{
	if (const int32* Value = Stats.Find(StatName))
	{
		return *Value;
	}
	return 0;
}

void UPBBaseStatComponent::SetStat(FName StatName, int32 Value)
{
	Stats.FindOrAdd(StatName) = Value;
}

void UPBBaseStatComponent::ApplyStat(FName StatName, int32 Delta)
{
	Stats.FindOrAdd(StatName) += Delta;
}