#include "PBBallSkillActorBase.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "EngineUtils.h"

void APBBallSkillActorBase::InitializeSkill(
	APBBallBase* InOwnerBall,
	const int32 InDamageAmount,
	const float InDuration,
	const int32 InDamageCount)
{
	static_cast<void>(InDamageAmount);
	static_cast<void>(InDuration);
	static_cast<void>(InDamageCount);

	OwnerBall = InOwnerBall;
	if (OwnerBall)
	{
		AttachToActor(OwnerBall, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		SetActorRelativeScale3D(FVector::OneVector);
		SetActorLocation(OwnerBall->GetActorLocation());
	}
}

void APBBallSkillActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(OwnerBall))
	{
		OwnerBall->OnDestroyed.AddUniqueDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}

	ChangeState(EPBBallSkillActorState::Preparing);
}

void APBBallSkillActorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(OwnerBall))
	{
		OwnerBall->OnDestroyed.RemoveDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}
	UnbindDamageEvents();
	Super::EndPlay(EndPlayReason);
}

void APBBallSkillActorBase::ActivateSkill()
{
	ChangeState(EPBBallSkillActorState::Active);
}

void APBBallSkillActorBase::FinishSkill()
{
	ChangeState(EPBBallSkillActorState::Finishing);
}

void APBBallSkillActorBase::StopSkill()
{
	ChangeState(EPBBallSkillActorState::Stopping);
}

void APBBallSkillActorBase::CompleteSkill()
{
	ChangeState(EPBBallSkillActorState::Completed);
}

bool APBBallSkillActorBase::ChangeState(const EPBBallSkillActorState NewState)
{
	if (State == NewState)
	{
		return false;
	}

	State = NewState;

	switch (NewState)
	{
	case EPBBallSkillActorState::Preparing:
		EnterPreparingState();
		break;
	case EPBBallSkillActorState::Active:
		EnterActiveState();
		break;
	case EPBBallSkillActorState::Finishing:
		EnterFinishingState();
		break;
	case EPBBallSkillActorState::Stopping:
		EnterStoppingState();
		break;
	case EPBBallSkillActorState::Completed:
		EnterCompletedState();
		break;
	default:
		break;
	}

	return true;
}

void APBBallSkillActorBase::PrepareSkill_Implementation()
{
	ActivateSkill();
}

void APBBallSkillActorBase::EnterPreparingState()
{
	PrepareSkill();
}

void APBBallSkillActorBase::EnterActiveState()
{
	OnActivated();
}

void APBBallSkillActorBase::EnterFinishingState()
{
	if (IsValid(OwnerBall))
	{
		OwnerBall->OnDestroyed.RemoveDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}
	OnFinished();
	if (!ShouldWaitForVisualCompletion())
	{
		CompleteSkill();
	}
}

void APBBallSkillActorBase::EnterStoppingState()
{
	if (IsValid(OwnerBall))
	{
		OwnerBall->OnDestroyed.RemoveDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}
	OnStopped();
	if (!ShouldWaitForVisualCompletion())
	{
		CompleteSkill();
	}
}

void APBBallSkillActorBase::EnterCompletedState()
{
	Destroy();
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
	OnHit(Target, AppliedDamage, HitLocation);
}

void APBBallSkillActorBase::HandleAreaDamageFinished()
{
	FinishSkill();
}

void APBBallSkillActorBase::HandleOwnerBallDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == OwnerBall)
	{
		StopSkill();
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
