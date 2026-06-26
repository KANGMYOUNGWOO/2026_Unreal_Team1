#include "PBBossWeaknessComponent.h"

UPBBossWeaknessComponent::UPBBossWeaknessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossWeaknessComponent::OpenWeakness()
{
	if (IsWeaknessOpen)
	{
		return;
	}

	IsWeaknessOpen = true;
	OnWeaknessOpenChanged.Broadcast(IsWeaknessOpen);

	UE_LOG(LogTemp, Warning, TEXT("Boss Weakness Opened."));
}

void UPBBossWeaknessComponent::CloseWeakness()
{
	if (!IsWeaknessOpen)
	{
		return;
	}

	IsWeaknessOpen = false;
	OnWeaknessOpenChanged.Broadcast(IsWeaknessOpen);

	UE_LOG(LogTemp, Warning, TEXT("Boss Weakness Closed."));
}

bool UPBBossWeaknessComponent::IsWeaknessPointOpen(FName WeaknessPointName) const
{
	return IsWeaknessOpen && IsWeaknessPoint(WeaknessPointName);
}

bool UPBBossWeaknessComponent::IsWeaknessPoint(FName WeaknessPointName) const
{
	return WeaknessPointDataMap.Contains(WeaknessPointName);
}

int32 UPBBossWeaknessComponent::CalculateWeaknessDamage(FName WeaknessPointName, int32 DamageAmount) const
{
	if (DamageAmount <= 0 || !IsWeaknessPointOpen(WeaknessPointName))
	{
		return DamageAmount;
	}

	const FBossWeaknessData* WeaknessData = WeaknessPointDataMap.Find(WeaknessPointName);
	if (!WeaknessData)
	{
		return DamageAmount;
	}

	return DamageAmount * WeaknessData->HPDamageMultiplierPercent / 100;
}
