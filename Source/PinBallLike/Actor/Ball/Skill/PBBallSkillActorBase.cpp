#include "PBBallSkillActorBase.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBSkillFeedbackComponent.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "EngineUtils.h"
#include "GameFramework/GameplayMessageSubsystem.h"

const FName APBBallSkillActorBase::SkillCollisionProfileName(TEXT("Skill"));

APBBallSkillActorBase::APBBallSkillActorBase()
{
	SkillFeedbackComponent = CreateDefaultSubobject<UPBSkillFeedbackComponent>(TEXT("SkillFeedbackComponent"));
}

void APBBallSkillActorBase::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	OwnerBall = InOwnerBall;
	SkillData = InSkillData;

	const UPBBaseStatComponent* StatComponent = OwnerBall
		? OwnerBall->FindComponentByClass<UPBBaseStatComponent>()
		: nullptr;
	const int32 BallAttackPower = StatComponent
		? StatComponent->GetStat(PBStatNames::Attack)
		: 0;
	SkillDamageAmount = FMath::Max(
		0,
		FMath::RoundToInt(static_cast<float>(BallAttackPower) * SkillData.PowerValue));

	if (OwnerBall)
	{
		AttachToActor(OwnerBall, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GetRootComponent()->SetAbsolute(false, true, false);
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
	if (SkillFeedbackComponent)
	{
		SkillFeedbackComponent->PlayCastFeedback();
	}
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
	if (SkillFeedbackComponent)
	{
		SkillFeedbackComponent->StopSkillSound();
	}
	OnFinished();
}

void APBBallSkillActorBase::EnterStoppingState()
{
	if (IsValid(OwnerBall))
	{
		OwnerBall->OnDestroyed.RemoveDynamic(this, &APBBallSkillActorBase::HandleOwnerBallDestroyed);
	}
	if (SkillFeedbackComponent)
	{
		SkillFeedbackComponent->StopSkillSound();
	}
	OnStopped();
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

void APBBallSkillActorBase::OnFinished_Implementation()
{
	CompleteSkill();
}

void APBBallSkillActorBase::OnStopped_Implementation()
{
	CompleteSkill();
}

void APBBallSkillActorBase::BindDamageEvents(UPBTimedAreaDamageComponent* DamageComponent)
{
	if (BoundDamageComponent)
	{
		BoundDamageComponent->OnAreaDamageApplied.RemoveAll(this);
		BoundDamageComponent->OnEffectFinished.RemoveAll(this);
	}

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
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBDamageLogMessage Message;
		Message.Style = EPBDamageLogStyle::PlayerSkill;
		Message.DamageAmount = AppliedDamage;
		Message.HitLocation = HitLocation;

		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_UI_DamageLog_Requested,
			Message);
	}

	if (SkillFeedbackComponent)
	{
		SkillFeedbackComponent->PlayHitFeedback();
	}

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
	if (BoundDamageComponent)
	{
		BoundDamageComponent->OnAreaDamageApplied.RemoveAll(this);
		BoundDamageComponent->OnEffectFinished.RemoveAll(this);
		BoundDamageComponent = nullptr;
	}

}
