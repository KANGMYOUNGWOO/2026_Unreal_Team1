#include "PBBossHitPartComponent.h"

#include "Components/PrimitiveComponent.h"

UPBBossHitPartComponent::UPBBossHitPartComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

EPBBossHitPartType UPBBossHitPartComponent::GetHitPartType() const
{
	return HitPartType;
}

FName UPBBossHitPartComponent::GetHitPointName() const
{
	return HitPointName;
}

void UPBBossHitPartComponent::GetHitCollisionComponents(TArray<UPrimitiveComponent*>& OutHitCollisionComponents) const
{
	OutHitCollisionComponents.Reset();

	TArray<USceneComponent*> ChildComponents;
	GetChildrenComponents(true, ChildComponents);

	for (USceneComponent* ChildComponent : ChildComponents)
	{
		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(ChildComponent))
		{
			OutHitCollisionComponents.Add(PrimitiveComponent);
		}
	}
}

bool UPBBossHitPartComponent::IsTargetHitComponent(const UPrimitiveComponent* PrimitiveComponent) const
{
	if (!PrimitiveComponent)
	{
		return false;
	}

	TArray<UPrimitiveComponent*> HitCollisionComponents;
	GetHitCollisionComponents(HitCollisionComponents);

	return HitCollisionComponents.Contains(PrimitiveComponent);
}
