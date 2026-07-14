#include "PBBallSkillActorBase.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "EngineUtils.h"

void APBBallSkillActorBase::ActivateEffect()
{
	if (bIsEnding || !IsValid(OwnerBall))
	{
		return;
	}

	OwnerBall->OnDestroyed.AddUniqueDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	BP_OnActivated();

	if (!ActivateEffectInternal())
	{
		OwnerBall->OnDestroyed.RemoveDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}
}

void APBBallSkillActorBase::DeactivateEffect()
{
	if (IsValid(OwnerBall))
	{
		OwnerBall->OnDestroyed.RemoveDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}

	DeactivateEffectInternal();
}

void APBBallSkillActorBase::FinishEffect()
{
	if (bIsEnding)
	{
		return;
	}

	bIsEnding = true;
	DeactivateEffect();
	BP_OnFinished();

	if (!ShouldWaitForVisualCompletion())
	{
		CompleteEffect();
	}
}

void APBBallSkillActorBase::StopEffect()
{
	if (bIsEnding)
	{
		return;
	}

	bIsEnding = true;
	DeactivateEffect();
	BP_OnStopped();

	if (!ShouldWaitForVisualCompletion())
	{
		CompleteEffect();
	}
}

void APBBallSkillActorBase::CompleteEffect()
{
	if (!bIsEnding || bIsCompleted)
	{
		return;
	}

	bIsCompleted = true;
	Destroy();
}

void APBBallSkillActorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateEffect();
	UnbindDamageEvents();
	Super::EndPlay(EndPlayReason);
}

bool APBBallSkillActorBase::ActivateEffectInternal()
{
	return true;
}

void APBBallSkillActorBase::DeactivateEffectInternal()
{
}

AActor* APBBallSkillActorBase::FindTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<APBBossBase> It(World); It; ++It)
	{
		APBBossBase* Boss = *It;
		if (IsTargetValid(Boss))
		{
			return Boss;
		}
	}

	return nullptr;
}

bool APBBallSkillActorBase::IsTargetValid(const AActor* Target) const
{
	const APBBossBase* Boss = Cast<APBBossBase>(Target);
	return IsValid(Boss) && !Boss->IsDead();
}

bool APBBallSkillActorBase::ShouldWaitForVisualCompletion() const
{
	return GetClass()->ClassGeneratedBy != nullptr;
}

void APBBallSkillActorBase::BindDamageEvents(UPBTimedAreaDamageComponent* DamageComponent)
{
	UnbindDamageEvents();
	BoundDamageComponent = DamageComponent;

	if (!BoundDamageComponent)
	{
		return;
	}

	BoundDamageComponent->OnAreaDamageApplied.AddUniqueDynamic(
		this,
		&APBBallSkillActorBase::HandleDamageApplied);
	BoundDamageComponent->OnEffectFinished.AddUniqueDynamic(
		this,
		&APBBallSkillActorBase::HandleAreaDamageFinished);
}

void APBBallSkillActorBase::HandleDamageApplied(
	AActor* Target,
	const int32 AppliedDamage,
	const FVector HitLocation)
{
	BP_OnHit(Target, AppliedDamage, HitLocation);
}

void APBBallSkillActorBase::HandleAreaDamageFinished()
{
	FinishEffect();
}

void APBBallSkillActorBase::HandleOwnerBallDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == OwnerBall)
	{
		StopEffect();
	}
}

void APBBallSkillActorBase::UnbindDamageEvents()
{
	if (!BoundDamageComponent)
	{
		return;
	}

	BoundDamageComponent->OnAreaDamageApplied.RemoveAll(this);
	BoundDamageComponent->OnEffectFinished.RemoveAll(this);
	BoundDamageComponent = nullptr;
}
