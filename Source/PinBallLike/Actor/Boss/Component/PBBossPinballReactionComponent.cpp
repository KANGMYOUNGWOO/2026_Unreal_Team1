#include "PBBossPinballReactionComponent.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Boss/Component/PBBossDamageComponent.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

UPBBossPinballReactionComponent::UPBBossPinballReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossPinballReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		if (UPBBossDamageComponent* DamageComponent = OwnerActor->FindComponentByClass<UPBBossDamageComponent>())
		{
			DamageComponent->OnDamageSourceHitApplied.AddUObject(this, &UPBBossPinballReactionComponent::HandleDamageSourceHitApplied);
		}
	}
}

void UPBBossPinballReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (UPBBossDamageComponent* DamageComponent = OwnerActor->FindComponentByClass<UPBBossDamageComponent>())
		{
			DamageComponent->OnDamageSourceHitApplied.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPBBossPinballReactionComponent::HandleDamageSourceHitApplied(AActor* DamageSource, const FHitResult& Hit)
{
	ApplyPinballHitImpulse(DamageSource, Hit);
	AddPinballCombo(DamageSource);
}

void UPBBossPinballReactionComponent::ApplyPinballHitImpulse(AActor* DamageSource, const FHitResult& Hit) const
{
	AActor* OwnerActor = GetOwner();
	if (!DamageSource || !OwnerActor || PinballHitImpulseStrength <= 0.0f)
	{
		return;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(DamageSource);
	if (!Movable)
	{
		return;
	}

	FVector ImpulseDirection = DamageSource->GetActorLocation() - OwnerActor->GetActorLocation();
	ImpulseDirection.Z = 0.0f;

	if (ImpulseDirection.IsNearlyZero())
	{
		ImpulseDirection = Hit.ImpactNormal;
		ImpulseDirection.Z = 0.0f;
	}

	ImpulseDirection = ImpulseDirection.GetSafeNormal();
	if (ImpulseDirection.IsNearlyZero())
	{
		return;
	}

	Movable->AddImpulse(ImpulseDirection * PinballHitImpulseStrength);
}

void UPBBossPinballReactionComponent::AddPinballCombo(AActor* DamageSource) const
{
	IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(DamageSource);
	if (!Comboable)
	{
		return;
	}

	Comboable->AddCombo(1);
}
