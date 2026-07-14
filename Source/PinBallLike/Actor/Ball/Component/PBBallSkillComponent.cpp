#include "PBBallSkillComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PinBallLike/Actor/Ball/Skill/PBCircularBladeActor.h"
#include "PinBallLike/Actor/Ball/Skill/PBCircularBladeSkill.h"

UPBBallSkillComponent::UPBBallSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBallSkillComponent::TryActivateSkill()
{
	return ActivateSkillActor() != nullptr;
}

APBBallSkillActorBase* UPBBallSkillComponent::ActivateSkillActor()
{
	if (IsValid(ActiveSkillActor))
	{
		return ActiveSkillActor;
	}

	APBBallBase* OwnerBall = Cast<APBBallBase>(GetOwner());
	UPBCircularBladeSkill* SkillInstance = GetOrCreateSkill();
	if (!OwnerBall || !SkillInstance)
	{
		return nullptr;
	}

	ActiveSkillActor = SkillInstance->Activate(OwnerBall, CircularBladeActorClass);
	if (ActiveSkillActor)
	{
		ActiveSkillActor->OnDestroyed.AddUniqueDynamic(
			this,
			&UPBBallSkillComponent::HandleActiveSkillActorDestroyed);
	}

	return ActiveSkillActor;
}

UPBCircularBladeSkill* UPBBallSkillComponent::GetOrCreateSkill()
{
	if (!Skill)
	{
		// TODO: Replace this test-only skill instance with a data-driven skill definition.
		Skill = NewObject<UPBCircularBladeSkill>(this, TEXT("Skill"));
	}

	return Skill;
}

void UPBBallSkillComponent::HandleActiveSkillActorDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == ActiveSkillActor)
	{
		ActiveSkillActor = nullptr;
	}
}
