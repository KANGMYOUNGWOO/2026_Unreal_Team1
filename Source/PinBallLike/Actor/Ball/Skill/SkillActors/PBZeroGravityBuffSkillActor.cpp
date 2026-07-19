#include "PBZeroGravityBuffSkillActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"

APBZeroGravityBuffSkillActor::APBZeroGravityBuffSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBZeroGravityBuffSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindStatusEffectEvents();
	Super::EndPlay(EndPlayReason);
}

void APBZeroGravityBuffSkillActor::EnterActiveState()
{
	if (!ApplyBuffToParty())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBZeroGravityBuffSkillActor::EnterFinishingState()
{
	UnbindStatusEffectEvents();
	Super::EnterFinishingState();
}

void APBZeroGravityBuffSkillActor::EnterStoppingState()
{
	UnbindStatusEffectEvents();
	RemoveAppliedBuffs();
	Super::EnterStoppingState();
}

APBCombatPartyController* APBZeroGravityBuffSkillActor::GetPartyController() const
{
	return IsValid(OwnerBall)
		? Cast<APBCombatPartyController>(OwnerBall->GetOwner())
		: nullptr;
}

bool APBZeroGravityBuffSkillActor::ApplyBuffToParty()
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
			|| !StatusEffectComponent->ApplyStatusEffect(PBStatusEffectAssetIds::StatusEffect::ZeroGravity))
		{
			continue;
		}

		StatusEffectComponent->OnStatusEffectRemoved.AddUniqueDynamic(
			this,
			&APBZeroGravityBuffSkillActor::HandleStatusEffectRemoved);
		AppliedStatusEffectComponents.Add(StatusEffectComponent);
	}

	return !AppliedStatusEffectComponents.IsEmpty();
}

void APBZeroGravityBuffSkillActor::RemoveAppliedBuffs()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->RemoveStatusEffect(PBStatusEffectAssetIds::StatusEffect::ZeroGravity);
		}
	}

	AppliedStatusEffectComponents.Reset();
}

void APBZeroGravityBuffSkillActor::UnbindStatusEffectEvents()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->OnStatusEffectRemoved.RemoveDynamic(
				this,
				&APBZeroGravityBuffSkillActor::HandleStatusEffectRemoved);
		}
	}
}

void APBZeroGravityBuffSkillActor::RemoveInactiveComponents()
{
	AppliedStatusEffectComponents.RemoveAll([](const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent)
	{
		return !StatusEffectComponent.IsValid()
			|| !StatusEffectComponent->HasStatusEffect(PBStatusEffectAssetIds::StatusEffect::ZeroGravity);
	});
}

void APBZeroGravityBuffSkillActor::HandleStatusEffectRemoved(
	const FName RemovedStatusEffectId,
	const int32 StackCount)
{
	if (RemovedStatusEffectId != PBStatusEffectAssetIds::StatusEffect::ZeroGravity)
	{
		return;
	}

	RemoveInactiveComponents();
	if (AppliedStatusEffectComponents.IsEmpty())
	{
		FinishSkill();
	}
}
