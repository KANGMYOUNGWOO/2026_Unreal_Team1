#include "PBBossBase.h"

#include "Component/PBBossDamageComponent.h"
#include "Component/PBBossDamageReceiverComponent.h"
#include "Component/PBBossGroggyComponent.h"
#include "Component/PBBossPatternComponent.h"
#include "Component/PBBossPinballReactionComponent.h"
#include "Component/PBBossStatComponent.h"
#include "Component/PBBossUIComponent.h"
#include "Component/PBBossWeaknessComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StateTreeComponent.h"
#include "PBBossStateTreeTags.h"
#include "PinBallLike/UI/Boss/PBBossStatusWidget.h"

namespace
{
	FGameplayTag GetBossStateRequestTag(EPBBossState BossState)
	{
		switch (BossState)
		{
		case EPBBossState::Idle:
			return PBBossStateTreeTags::RequestIdle;
		case EPBBossState::Pattern:
			return PBBossStateTreeTags::RequestPattern;
		case EPBBossState::Groggy:
			return PBBossStateTreeTags::RequestGroggy;
		case EPBBossState::Enraged:
			return PBBossStateTreeTags::RequestEnraged;
		case EPBBossState::Dead:
			return PBBossStateTreeTags::RequestDead;
		default:
			return FGameplayTag();
		}
	}
}

APBBossBase::APBBossBase()
{
	PrimaryActorTick.bCanEverTick = false;
	BossName = NSLOCTEXT("Boss", "DefaultBossName", "Boss");

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(100.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetGenerateOverlapEvents(false);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	BossStatComponent = CreateDefaultSubobject<UPBBossStatComponent>(TEXT("BossStatComponent"));
	BossGroggyComponent = CreateDefaultSubobject<UPBBossGroggyComponent>(TEXT("BossGroggyComponent"));
	BossDamageComponent = CreateDefaultSubobject<UPBBossDamageComponent>(TEXT("BossDamageComponent"));
	BossDamageReceiverComponent = CreateDefaultSubobject<UPBBossDamageReceiverComponent>(TEXT("BossDamageReceiverComponent"));
	BossPatternComponent = CreateDefaultSubobject<UPBBossPatternComponent>(TEXT("BossPatternComponent"));
	BossPinballReactionComponent = CreateDefaultSubobject<UPBBossPinballReactionComponent>(TEXT("BossPinballReactionComponent"));
	BossWeaknessComponent = CreateDefaultSubobject<UPBBossWeaknessComponent>(TEXT("BossWeaknessComponent"));
	BossStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("BossStateTreeComponent"));
	BossStateTreeComponent->SetStartLogicAutomatically(false);
	BossUIComponent = CreateDefaultSubobject<UPBBossUIComponent>(TEXT("BossUIComponent"));
}

UPBBossStatComponent* APBBossBase::GetBossStatComponent() const
{
	return BossStatComponent;
}

UPBBossGroggyComponent* APBBossBase::GetBossGroggyComponent() const
{
	return BossGroggyComponent;
}

UPBBossDamageComponent* APBBossBase::GetBossDamageComponent() const
{
	return BossDamageComponent;
}

UPBBossPatternComponent* APBBossBase::GetBossPatternComponent() const
{
	return BossPatternComponent;
}

UPBBossWeaknessComponent* APBBossBase::GetBossWeaknessComponent() const
{
	return BossWeaknessComponent;
}

UStateTreeComponent* APBBossBase::GetBossStateTreeComponent() const
{
	return BossStateTreeComponent;
}

UPBBossUIComponent* APBBossBase::GetBossUIComponent() const
{
	return BossUIComponent;
}

void APBBossBase::SetBossState(EPBBossState NewBossState)
{
	BossState = NewBossState;
}

void APBBossBase::RequestBossState(EPBBossState NewBossState)
{
	const FGameplayTag StateRequestTag = GetBossStateRequestTag(NewBossState);
	if (BossStateTreeComponent && StateRequestTag.IsValid())
	{
		BossStateTreeComponent->SendStateTreeEvent(StateRequestTag);
	}
}

EPBBossState APBBossBase::GetBossState() const
{
	return BossState;
}

bool APBBossBase::IsIdleState() const
{
	return BossState == EPBBossState::Idle;
}

bool APBBossBase::IsPatternState() const
{
	return BossState == EPBBossState::Pattern;
}

bool APBBossBase::IsGroggyState() const
{
	return BossState == EPBBossState::Groggy;
}

bool APBBossBase::IsEnragedState() const
{
	return BossState == EPBBossState::Enraged;
}

bool APBBossBase::IsDeadState() const
{
	return BossState == EPBBossState::Dead;
}

bool APBBossBase::IsEnragedPhase() const
{
	return BossStatComponent && BossStatComponent->IsEnraged;
}

bool APBBossBase::IsFixedBoss() const
{
	return BossMovementType == EPBBossMovementType::Fixed;
}

bool APBBossBase::IsMovableBoss() const
{
	return BossMovementType == EPBBossMovementType::Movable;
}

void APBBossBase::StartIdleState_Implementation()
{
	SetBossState(EPBBossState::Idle);
}

void APBBossBase::StartPatternState()
{
	if (BossPatternComponent)
	{
		BossPatternComponent->StartPatternSystem();
	}
}

void APBBossBase::StopPatternState()
{
	if (BossPatternComponent)
	{
		BossPatternComponent->StopPatternSystem();
	}
}

void APBBossBase::StartGroggyState()
{
	if (IsGroggyStateActive || IsDead())
	{
		return;
	}

	IsGroggyStateActive = true;
	SetBossState(EPBBossState::Groggy);

	if (BossPatternComponent)
	{
		if (BossPatternComponent->GetCurrentPattern())
		{
			UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Cancel Current Pattern."));
		}
		BossPatternComponent->PausePatternSystem();
	}

	SetWeaknessState(true);

	StartGroggyResetTimer();
	BP_OnGroggyStarted();
}

void APBBossBase::FinishGroggyState()
{
	if (!IsGroggyStateActive || !BossGroggyComponent || IsDead())
	{
		return;
	}

	IsGroggyStateActive = false;
	SetWeaknessState(false);
	ClearGroggyResetTimer();

	BossGroggyComponent->ResetGroggy();

	if (BossPatternComponent && BossPatternComponent->ResumePatternSystem())
	{
		UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Finished. Restart Pattern System."));
	}
}

void APBBossBase::StartEnragedState()
{
	SetBossState(EPBBossState::Enraged);

	if (BossPatternComponent)
	{
		BossPatternComponent->NotifyEnragedPhaseStarted();
	}

	BP_OnEnragedStarted();
}

void APBBossBase::StartDeadState()
{
	IsGroggyStateActive = false;
	SetBossState(EPBBossState::Dead);

	if (BossPatternComponent)
	{
		BossPatternComponent->StopPatternSystem();
	}

	SetWeaknessState(false);

	ClearGroggyResetTimer();
	BP_OnDead();
}

FText APBBossBase::GetBossName() const
{
	return BossName;
}

void APBBossBase::SetPinballCollisionDamageBlocked(bool IsBlocked)
{
	if (BossDamageComponent)
	{
		BossDamageComponent->SetPinballCollisionDamageBlocked(IsBlocked);
	}
}

bool APBBossBase::IsPinballCollisionDamageBlocked() const
{
	return BossDamageComponent && BossDamageComponent->IsPinballCollisionDamageBlocked();
}

void APBBossBase::NotifyBossDamaged(FName HitPointName, int32 DamageAmount)
{
	BP_OnDamaged(HitPointName, DamageAmount);
}

void APBBossBase::BeginPlay()
{
	Super::BeginPlay();

	SetBossState(EPBBossState::Idle);
	SetWeaknessState(false);

	if (BossStateTreeComponent)
	{
		BossStateTreeComponent->StartLogic();
	}

	if (BossUIComponent && BossStatusWidgetClass)
	{
		BossUIComponent->ConfigureBossStatusWidget(BossStatusWidgetClass, BossStatusWidgetZOrder);
	}
}

void APBBossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetWeaknessState(false);

	if (BossStateTreeComponent)
	{
		BossStateTreeComponent->StopLogic(TEXT("Boss EndPlay"));
	}

	ClearGroggyResetTimer();
	Super::EndPlay(EndPlayReason);
}

void APBBossBase::TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount)
{
	if (!BossDamageComponent)
	{
		return;
	}

	BossDamageComponent->ApplyPointDamage(GroggyPointName, DamageAmount);
}

void APBBossBase::OnGroggyTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Started."));
	RequestBossState(EPBBossState::Groggy);
}

void APBBossBase::OnEnragedTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Enraged Started."));
	RequestBossState(EPBBossState::Enraged);
}

void APBBossBase::OnDeadTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Dead."));
	RequestBossState(EPBBossState::Dead);
}

bool APBBossBase::IsDead() const
{
	return BossStatComponent ? BossStatComponent->IsDead() : true;
}

void APBBossBase::StartGroggyResetTimer()
{
	if (!BossGroggyComponent)
	{
		return;
	}

	if (GroggyDurationSeconds <= 0.0f)
	{
		HandleGroggyDurationFinished();
		return;
	}

	GetWorldTimerManager().SetTimer(
		GroggyResetTimerHandle,
		this,
		&APBBossBase::HandleGroggyDurationFinished,
		GroggyDurationSeconds,
		false);
}

void APBBossBase::ClearGroggyResetTimer()
{
	GetWorldTimerManager().ClearTimer(GroggyResetTimerHandle);
}

void APBBossBase::HandleGroggyDurationFinished()
{
	if (!BossGroggyComponent || IsDead())
	{
		return;
	}

	RequestBossState(EPBBossState::Idle);
}

void APBBossBase::SetWeaknessState(bool IsOpen)
{
	if (BossWeaknessComponent)
	{
		if (IsOpen)
		{
			BossWeaknessComponent->OpenWeakness();
		}
		else
		{
			BossWeaknessComponent->CloseWeakness();
		}
	}
}
