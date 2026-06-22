// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallStatComponent.h"

UPBBallStatComponent::UPBBallStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Stats.SetNumZeroed(static_cast<int32>(EBallStatType::EST_MAX));
}

void UPBBallStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

int32 UPBBallStatComponent::GetStat(EBallStatType InType) const
{
	const int32 Idx = StatTypeToIndex(InType);
	return Stats.IsValidIndex(Idx) ? Stats[Idx] : 0;
}

void UPBBallStatComponent::SetStat(EBallStatType InType, int32 Value)
{
	const int32 Idx = StatTypeToIndex(InType);
	if (Stats.IsValidIndex(Idx))
	{
		Stats[Idx] = Value;
	}
}

void UPBBallStatComponent::ApplyStat(EBallStatType InType, int32 Delta)
{
	const int32 Idx = StatTypeToIndex(InType);
	if (Stats.IsValidIndex(Idx))
	{
		Stats[Idx] += Delta;
	}
}


int32 UPBBallStatComponent::StatTypeToIndex(EBallStatType InType) const
{
	const int32 Idx = static_cast<int32>(InType);
	return Idx;
}
