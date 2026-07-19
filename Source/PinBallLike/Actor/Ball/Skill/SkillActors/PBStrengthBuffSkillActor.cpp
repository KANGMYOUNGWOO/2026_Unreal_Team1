#include "PBStrengthBuffSkillActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"

APBStrengthBuffSkillActor::APBStrengthBuffSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBStrengthBuffSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindStatusEffectEvents();
	Super::EndPlay(EndPlayReason);
}

void APBStrengthBuffSkillActor::EnterActiveState()
{
	if (!ApplyBuffToParty())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBStrengthBuffSkillActor::EnterFinishingState()
{
	UnbindStatusEffectEvents();
	Super::EnterFinishingState();
}

void APBStrengthBuffSkillActor::EnterStoppingState()
{
	UnbindStatusEffectEvents();

	// TODO: 시전자 사망시 버프 제거 할건지에 대한 고민 필요
	RemoveAppliedBuffs();
	Super::EnterStoppingState();
}

APBCombatPartyController* APBStrengthBuffSkillActor::GetPartyController() const
{
	return IsValid(OwnerBall)
		? Cast<APBCombatPartyController>(OwnerBall->GetOwner())
		: nullptr;
}

bool APBStrengthBuffSkillActor::ApplyBuffToParty()
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
			&APBStrengthBuffSkillActor::HandleStatusEffectRemoved);
		AppliedStatusEffectComponents.Add(StatusEffectComponent);
	}

	return !AppliedStatusEffectComponents.IsEmpty();
}

void APBStrengthBuffSkillActor::RemoveAppliedBuffs()
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

void APBStrengthBuffSkillActor::UnbindStatusEffectEvents()
{
	for (const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent : AppliedStatusEffectComponents)
	{
		if (StatusEffectComponent.IsValid())
		{
			StatusEffectComponent->OnStatusEffectRemoved.RemoveDynamic(
				this,
				&APBStrengthBuffSkillActor::HandleStatusEffectRemoved);
		}
	}
}

void APBStrengthBuffSkillActor::RemoveInactiveComponents()
{
	AppliedStatusEffectComponents.RemoveAll([this](const TWeakObjectPtr<UPBStatusEffectComponent>& StatusEffectComponent)
	{
		return !StatusEffectComponent.IsValid()
			|| !StatusEffectComponent->HasStatusEffect(StatusEffectId);
	});
}

void APBStrengthBuffSkillActor::HandleStatusEffectRemoved(
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
