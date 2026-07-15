#include "PBPartyBuffSkillActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"

APBPartyBuffSkillActor::APBPartyBuffSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBPartyBuffSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindStatusEffectEvents();
	Super::EndPlay(EndPlayReason);
}

void APBPartyBuffSkillActor::EnterActiveState()
{
	if (!ApplyBuffToParty())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBPartyBuffSkillActor::EnterFinishingState()
{
	UnbindStatusEffectEvents();
	Super::EnterFinishingState();
}

void APBPartyBuffSkillActor::EnterStoppingState()
{
	UnbindStatusEffectEvents();

	// TODO: 시전자 사망시 버프 제거 할건지에 대한 고민 필요
	RemoveAppliedBuffs();
	Super::EnterStoppingState();
}

APBCombatPartyController* APBPartyBuffSkillActor::GetPartyController() const
{
	return IsValid(OwnerBall)
		? Cast<APBCombatPartyController>(OwnerBall->GetOwner())
		: nullptr;
}

bool APBPartyBuffSkillActor::ApplyBuffToParty()
{
	APBCombatPartyController* PartyController = GetPartyController();
	if (!IsValid(PartyController) || StatusEffectId.IsNone())
	{
		return false;
	}

	AppliedStatusEffectComponents.Reset();
	for (APBBallBase* PartyBall : PartyController->GetValidPartyBalls())
	{
		UPBStatusEffectComponent* StatusEffectComponent = IsValid(PartyBall)
			? PartyBall->GetStatusEffectComponent()
			: nullptr;
		if (!StatusEffectComponent || !StatusEffectComponent->ApplyStatusEffect(StatusEffectId))
		{
			continue;
		}

		StatusEffectComponent->OnStatusEffectRemoved.AddUniqueDynamic(
			this,
			&APBPartyBuffSkillActor::HandleStatusEffectRemoved);
		AppliedStatusEffectComponents.Add(StatusEffectComponent);
	}

	return !AppliedStatusEffectComponents.IsEmpty();
}

void APBPartyBuffSkillActor::RemoveAppliedBuffs()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->RemoveStatusEffect(StatusEffectId);
		}
	}

	AppliedStatusEffectComponents.Reset();
}

void APBPartyBuffSkillActor::UnbindStatusEffectEvents()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->OnStatusEffectRemoved.RemoveDynamic(
				this,
				&APBPartyBuffSkillActor::HandleStatusEffectRemoved);
		}
	}
}

void APBPartyBuffSkillActor::RemoveInactiveComponents()
{
	AppliedStatusEffectComponents.RemoveAll([this](const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent)
	{
		return !StatusEffectComponent.IsValid()
			|| !StatusEffectComponent->HasStatusEffect(StatusEffectId);
	});
}

void APBPartyBuffSkillActor::HandleStatusEffectRemoved(
	const FName RemovedStatusEffectId,
	const int32 StackCount)
{
	if (RemovedStatusEffectId != StatusEffectId)
	{
		return;
	}

	RemoveInactiveComponents();
	if (AppliedStatusEffectComponents.IsEmpty())
	{
		FinishSkill();
	}
}
