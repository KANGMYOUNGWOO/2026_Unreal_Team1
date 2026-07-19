#include "PinBallLike/Actor/StatusEffect/Effects/PBZeroGravityStatusEffect.h"

#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"

void UPBZeroGravityStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
	AActor* OwnerActor = GetOwnerActor();
	UPBBallPhysicsComponent* PhysicsComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
	if (!PhysicsComponent)
	{
		return;
	}

	if (TriggerEvent == PBStatusEffectTriggerEvents::Applied)
	{
		PhysicsComponent->AddGravityDisableRequest(this);
		return;
	}

	if (TriggerEvent == PBStatusEffectTriggerEvents::Removed
		|| TriggerEvent == PBStatusEffectTriggerEvents::Expired)
	{
		PhysicsComponent->RemoveGravityDisableRequest(this);
	}
}
