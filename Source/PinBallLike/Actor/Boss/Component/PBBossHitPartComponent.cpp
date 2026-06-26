#include "PBBossHitPartComponent.h"

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

UPrimitiveComponent* UPBBossHitPartComponent::GetHitCollisionComponent() const
{
	return HitCollisionComponent;
}

bool UPBBossHitPartComponent::IsTargetHitComponent(const UPrimitiveComponent* PrimitiveComponent) const
{
	return PrimitiveComponent && HitCollisionComponent == PrimitiveComponent;
}
