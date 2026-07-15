#include "PBBossBase.h"

#include "Component/PBBossDamageComponent.h"
#include "Component/PBBossGroggyComponent.h"
#include "Component/PBBossHitEffectComponent.h"
#include "Component/PBBossIntroComponent.h"
#include "Component/PBBossPatternComponent.h"
#include "Component/PBBossPinballReactionComponent.h"
#include "Component/PBBossStatComponent.h"
#include "Component/PBBossUIComponent.h"
#include "Component/PBBossWeaknessComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "PinBallLike/Actor/Boss/StateTree/PBBossStateTreeTags.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Table/Boss/DataAsset/PBBossDataAsset.h"

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
	BossHitEffectComponent = CreateDefaultSubobject<UPBBossHitEffectComponent>(TEXT("BossHitEffectComponent"));
	BossPatternComponent = CreateDefaultSubobject<UPBBossPatternComponent>(TEXT("BossPatternComponent"));
	BossIntroComponent = CreateDefaultSubobject<UPBBossIntroComponent>(TEXT("BossIntroComponent"));
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

UPBBossIntroComponent* APBBossBase::GetBossIntroComponent() const
{
	return BossIntroComponent;
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
	if (IsGroggyState() || IsDead())
	{
		return;
	}

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
	if (!IsGroggyState() || !BossGroggyComponent || IsDead())
	{
		return;
	}

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

	if (BossUIComponent)
	{
		BossUIComponent->ShowEnrageWarning();
	}

	if (EnrageCameraShakeClass)
	{
		UWorld* World = GetWorld();
		APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
		if (PlayerController)
		{
			PlayerController->ClientStartCameraShake(EnrageCameraShakeClass);
		}
	}

	BP_OnEnragedStarted();

	if (!IsDead())
	{
		RequestBossState(EPBBossState::Idle);
	}
}

void APBBossBase::StartDeadState()
{
	if (IsDeadState())
	{
		return;
	}

	SetBossState(EPBBossState::Dead);

	if (BossPatternComponent)
	{
		BossPatternComponent->StopPatternSystem();
	}

	SetWeaknessState(false);

	ClearGroggyResetTimer();
	BP_OnDead();

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBBattleBossDeadMessage Message;
		Message.BossActor = this;

		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_Battle_Boss_Dead,
			Message);
	}
}

FText APBBossBase::GetBossName() const
{
	return BossName;
}

UTexture2D* APBBossBase::GetBossIntroImage() const
{
	return BossIntroImage;
}

void APBBossBase::InitializeFromBossDataAsset(const UPBBossDataAsset* BossDataAsset)
{
	if (!IsValid(BossDataAsset))
	{
		return;
	}

	if (!BossDataAsset->BossName.IsEmpty())
	{
		BossName = BossDataAsset->BossName;
	}

	BossMovementType = BossDataAsset->BossMovementType;
	GroggyDurationSeconds = FMath::Max(0.1f, BossDataAsset->GroggyDurationSeconds);
	EnrageCameraShakeClass = BossDataAsset->EnrageCameraShakeClass.Get();

	if (BossStatComponent)
	{
		BossStatComponent->MaxHP = FMath::Max(1, BossDataAsset->MaxHP);
		BossStatComponent->EnrageHPRatioPercent = FMath::Clamp(BossDataAsset->EnrageHPRatioPercent, 0, 100);
		BossStatComponent->HitPointDataMap.Reset();

		for (const FPBBossHitPointData& HitPointData : BossDataAsset->HitPointDatas)
		{
			if (HitPointData.HitPointName.IsNone())
			{
				continue;
			}

			FBossHitPointDamageData DamageData;
			DamageData.HPDamageMultiplierPercent = FMath::Max(0, HitPointData.HPDamageMultiplierPercent);
			BossStatComponent->HitPointDataMap.Add(HitPointData.HitPointName, DamageData);
		}
	}

	if (BossGroggyComponent)
	{
		BossGroggyComponent->MaxGroggyGauge = FMath::Max(1, BossDataAsset->MaxGroggyGauge);
		BossGroggyComponent->GroggyPointDataMap.Reset();

		for (const FPBBossHitPointData& HitPointData : BossDataAsset->HitPointDatas)
		{
			if (HitPointData.HitPointName.IsNone())
			{
				continue;
			}

			FBossGroggyPointData GroggyData;
			GroggyData.GroggyMultiplierPercent = FMath::Max(0, HitPointData.GroggyMultiplierPercent);
			BossGroggyComponent->GroggyPointDataMap.Add(HitPointData.HitPointName, GroggyData);
		}
	}

	if (BossDamageComponent)
	{
		BossDamageComponent->ConfigureDamageSettings(
			BossDataAsset->DefaultHitPointName,
			BossDataAsset->DamageCooldownSeconds);
	}

	if (BossPatternComponent)
	{
		BossPatternComponent->ConfigurePatternData(
			BossDataAsset->PatternDatas,
			BossDataAsset->EnragedPatternDatas,
			BossDataAsset->EnragedEntryPatternDatas,
			BossDataAsset->MinPatternIntervalSeconds,
			BossDataAsset->PatternCheckIntervalSeconds);
	}

	if (BossWeaknessComponent)
	{
		BossWeaknessComponent->WeaknessPointNames.Reset();

		for (const FPBBossHitPointData& HitPointData : BossDataAsset->HitPointDatas)
		{
			if (HitPointData.IsWeaknessPoint && !HitPointData.HitPointName.IsNone())
			{
				BossWeaknessComponent->WeaknessPointNames.Add(HitPointData.HitPointName);
			}
		}
	}
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
	if (BossHitEffectComponent)
	{
		BossHitEffectComponent->PlayHitEffect();
	}

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

void APBBossBase::DamageToBoss_Implementation(int32 DamageAmount)
{
	if (!BossDamageComponent)
	{
		return;
	}

	BossDamageComponent->ApplyHitPartDamage(DamageAmount);
}

void APBBossBase::OnGroggyTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Started."));
	RequestBossState(EPBBossState::Groggy);
}

void APBBossBase::IncreaseGroggy_Implementation(int32 GroggyAmount)
{
	if (!BossGroggyComponent || IsDead())
	{
		return;
	}

	BossGroggyComponent->ApplyGroggyDamage(GroggyAmount);
}

void APBBossBase::OnEnragedTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Enraged Started."));
	RequestBossState(EPBBossState::Enraged);
}

void APBBossBase::OnDeadTriggered_Implementation()
{
	HandleDeadTriggered();
}

void APBBossBase::HandleDeadTriggered()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Dead."));

	if (BossStateTreeComponent && BossStateTreeComponent->IsRunning())
	{
		RequestBossState(EPBBossState::Dead);
		return;
	}

	StartDeadState();
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

