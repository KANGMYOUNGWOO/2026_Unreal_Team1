// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallComboComponent.h"


UPBBallComboComponent::UPBBallComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UPBBallComboComponent::GetCombo() const
{
	return CurrentCombo;
}

int32 UPBBallComboComponent::GetMaxCombo() const
{
	return MaxCombo;
}

void UPBBallComboComponent::SetCombo(int32 Value)
{
	const int32 NewCombo = FMath::Max(0, Value);
	if (CurrentCombo == NewCombo)
	{
		return;
	}

	CurrentCombo = NewCombo;
	MaxCombo = FMath::Max(MaxCombo, CurrentCombo);
	OnComboChanged.Broadcast(CurrentCombo, MaxCombo);
}

void UPBBallComboComponent::AddCombo(int32 Delta)
{
	if (Delta == 0)
	{
		return;
	}

	SetCombo(CurrentCombo + Delta);
}

bool UPBBallComboComponent::TryConsumeCombo(int32 Cost)
{
	if (Cost < 0 || CurrentCombo < Cost)
	{
		return false;
	}

	SetCombo(CurrentCombo - Cost);
	return true;
}

void UPBBallComboComponent::ResetCombo()
{
	SetCombo(0);
}
