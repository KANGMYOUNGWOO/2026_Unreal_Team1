#include "PBBossWeaknessComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Boss/Component/PBBossHitPartComponent.h"

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

	TArray<UPBBossHitPartComponent*> HitPartComponents;
	OwnerActor->GetComponents<UPBBossHitPartComponent>(HitPartComponents);

	for (const UPBBossHitPartComponent* HitPartComponent : HitPartComponents)
	{
		if (!IsWeaknessHitPart(HitPartComponent))
		{
			continue;
		}

		TArray<UPrimitiveComponent*> HitCollisionComponents;
		HitPartComponent->GetHitCollisionComponents(HitCollisionComponents);

		for (UPrimitiveComponent* PrimitiveComponent : HitCollisionComponents)
		{
			ApplyCollisionState(PrimitiveComponent, IsEnabled);
		}
	}
}

bool UPBBossWeaknessComponent::IsWeaknessHitPart(const UPBBossHitPartComponent* HitPartComponent) const
{
	return HitPartComponent
		&& HitPartComponent->GetHitPartType() == EPBBossHitPartType::WeakPoint
		&& IsWeaknessPoint(HitPartComponent->GetHitPointName());
}

void UPBBossWeaknessComponent::ApplyCollisionState(UPrimitiveComponent* PrimitiveComponent, bool IsEnabled)
{
	if (!PrimitiveComponent)
	{
		return;
	}

	const TObjectKey<UPrimitiveComponent> PrimitiveComponentKey(PrimitiveComponent);
	if (!WeaknessCollisionEnabledMap.Contains(PrimitiveComponentKey))
	{
		WeaknessCollisionEnabledMap.Add(PrimitiveComponentKey, PrimitiveComponent->GetCollisionEnabled());
	}

	if (PrimitiveComponent->IsA<UCapsuleComponent>())
	{
		PrimitiveComponent->SetHiddenInGame(true);
	}
	else
	{
		PrimitiveComponent->SetHiddenInGame(!IsEnabled);
		PrimitiveComponent->SetVisibility(IsEnabled);
	}

	if (IsEnabled)
	{
		const ECollisionEnabled::Type* OriginalCollisionEnabled = WeaknessCollisionEnabledMap.Find(PrimitiveComponentKey);
		PrimitiveComponent->SetCollisionEnabled(OriginalCollisionEnabled ? *OriginalCollisionEnabled : ECollisionEnabled::QueryAndPhysics);
		return;
	}

	if (PrimitiveComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		WeaknessCollisionEnabledMap.FindOrAdd(PrimitiveComponentKey) = PrimitiveComponent->GetCollisionEnabled();
	}

	PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
