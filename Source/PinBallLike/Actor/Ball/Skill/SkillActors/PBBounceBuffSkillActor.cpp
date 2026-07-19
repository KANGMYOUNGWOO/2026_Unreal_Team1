#include "PBBounceBuffSkillActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"

APBBounceBuffSkillActor::APBBounceBuffSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBBounceBuffSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindStatusEffectEvents();
	Super::EndPlay(EndPlayReason);
}

void APBBounceBuffSkillActor::EnterActiveState()
{
	if (!ApplyBuffToParty())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBBounceBuffSkillActor::EnterFinishingState()
{
	UnbindStatusEffectEvents();
	Super::EnterFinishingState();
}

void APBBounceBuffSkillActor::EnterStoppingState()
{
	UnbindStatusEffectEvents();
	RemoveAppliedBuffs();
	Super::EnterStoppingState();
}

APBCombatPartyController* APBBounceBuffSkillActor::GetPartyController() const
{
	return IsValid(OwnerBall)
		? Cast<APBCombatPartyController>(OwnerBall->GetOwner())
		: nullptr;
}

bool APBBounceBuffSkillActor::ApplyBuffToParty()
{
	APBCombatPartyController* PartyController = GetPartyController();
	if (!IsValid(PartyController))
	{
		return false;
	}

	AppliedStatusEffectComponents.Reset();
	for (APBBallBase* PartyBall : PartyController->GetValidPartyBalls())
	{
		UPBStatusEffectComponent* StatusEffectComponent = IsValid(PartyBall)
			? PartyBall->GetStatusEffectComponent()
			: nullptr;
		if (!StatusEffectComponent
			|| !StatusEffectComponent->ApplyStatusEffect(PBStatusEffectAssetIds::StatusEffect::BounceUp))
		{
			continue;
		}

		StatusEffectComponent->OnStatusEffectRemoved.AddUniqueDynamic(
			this,
			&APBBounceBuffSkillActor::HandleStatusEffectRemoved);
		AppliedStatusEffectComponents.Add(StatusEffectComponent);
	}

	return !AppliedStatusEffectComponents.IsEmpty();
}

void APBBounceBuffSkillActor::RemoveAppliedBuffs()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->RemoveStatusEffect(PBStatusEffectAssetIds::StatusEffect::BounceUp);
		}
	}

	AppliedStatusEffectComponents.Reset();
}

void APBBounceBuffSkillActor::UnbindStatusEffectEvents()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->OnStatusEffectRemoved.RemoveDynamic(
				this,
				&APBBounceBuffSkillActor::HandleStatusEffectRemoved);
		}
	}
}

void APBBounceBuffSkillActor::RemoveInactiveComponents()
{
	AppliedStatusEffectComponents.RemoveAll([](const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent)
	{
		return !StatusEffectComponent.IsValid()
			|| !StatusEffectComponent->HasStatusEffect(PBStatusEffectAssetIds::StatusEffect::BounceUp);
	});
}

void APBBounceBuffSkillActor::HandleStatusEffectRemoved(
	const FName RemovedStatusEffectId,
	const int32 StackCount)
{
	if (RemovedStatusEffectId != PBStatusEffectAssetIds::StatusEffect::BounceUp)
	{
		return;
	}

	RemoveInactiveComponents();
	if (AppliedStatusEffectComponents.IsEmpty())
	{
		FinishSkill();
	}
}
