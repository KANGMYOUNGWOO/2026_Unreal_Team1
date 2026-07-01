#include "PBBossWeaknessComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UPBBossWeaknessComponent::UPBBossWeaknessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossWeaknessComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyWeaknessCollisionState(IsWeaknessOpen);
}

void UPBBossWeaknessComponent::OpenWeakness()
{
	if (IsWeaknessOpen)
	{
		ApplyWeaknessCollisionState(true);
		return;
	}

	IsWeaknessOpen = true;
	ApplyWeaknessCollisionState(true);
	OnWeaknessOpenChanged.Broadcast(IsWeaknessOpen);

	UE_LOG(LogTemp, Warning, TEXT("Boss Weakness Opened."));
}

void UPBBossWeaknessComponent::CloseWeakness()
{
	if (!IsWeaknessOpen)
	{
		ApplyWeaknessCollisionState(false);
		return;
	}

	IsWeaknessOpen = false;
	ApplyWeaknessCollisionState(false);
	OnWeaknessOpenChanged.Broadcast(IsWeaknessOpen);

	UE_LOG(LogTemp, Warning, TEXT("Boss Weakness Closed."));
}

bool UPBBossWeaknessComponent::IsWeaknessPointOpen(FName WeaknessPointName) const
{
	return IsWeaknessOpen && IsWeaknessPoint(WeaknessPointName);
}

bool UPBBossWeaknessComponent::IsWeaknessPoint(FName WeaknessPointName) const
{
	return WeaknessPointNames.Contains(WeaknessPointName);
}

void UPBBossWeaknessComponent::ApplyWeaknessCollisionState(bool IsEnabled)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent || !IsWeaknessCollisionComponent(PrimitiveComponent))
		{
			continue;
		}

		const TObjectKey<UPrimitiveComponent> PrimitiveComponentKey(PrimitiveComponent);
		if (!WeaknessCollisionEnabledMap.Contains(PrimitiveComponentKey))
		{
			WeaknessCollisionEnabledMap.Add(PrimitiveComponentKey, PrimitiveComponent->GetCollisionEnabled());
		}

		PrimitiveComponent->SetHiddenInGame(!IsEnabled, true);
		PrimitiveComponent->SetVisibility(IsEnabled, true);

		if (IsEnabled)
		{
			const ECollisionEnabled::Type* OriginalCollisionEnabled = WeaknessCollisionEnabledMap.Find(PrimitiveComponentKey);
			PrimitiveComponent->SetCollisionEnabled(OriginalCollisionEnabled ? *OriginalCollisionEnabled : ECollisionEnabled::QueryAndPhysics);
			continue;
		}

		if (PrimitiveComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			WeaknessCollisionEnabledMap.FindOrAdd(PrimitiveComponentKey) = PrimitiveComponent->GetCollisionEnabled();
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool UPBBossWeaknessComponent::IsWeaknessCollisionComponent(const UPrimitiveComponent* PrimitiveComponent) const
{
	if (!PrimitiveComponent)
	{
		return false;
	}

	for (const FName ComponentTag : PrimitiveComponent->ComponentTags)
	{
		if (IsWeaknessPoint(ComponentTag))
		{
			return true;
		}
	}

	return false;
}
