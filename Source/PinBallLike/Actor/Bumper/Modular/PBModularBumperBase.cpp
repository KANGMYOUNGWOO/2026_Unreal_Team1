// Fill out your copyright notice in the Description page of Project Settings.


#include "PBModularBumperBase.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBBumperPositionAnchor.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBModularBumperBase::APBModularBumperBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBModularBumperBase::BeginPlay()
{
	Super::BeginPlay();

	CreateBumperEffect();
	NotifyTriggerCountChanged();

	for (auto spawnInfo : TriggerSpawnInfos)
	{
		SpawnTriggerActorsFromInfo(spawnInfo);
	}
}

void APBModularBumperBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearTriggerActors();

	Super::EndPlay(EndPlayReason);
}

void APBModularBumperBase::HandleTriggerActorActivated(
	APBBumperTriggerActorBase* TriggerActor,
	APBBallBase* Ball,
	const FHitResult& TriggerHit)
{
	if (!IsValid(TriggerActor) || !IsValid(Ball) || !CanAccumulateTrigger())
	{
		return;
	}

	AddTriggerCount(Ball);
}

void APBModularBumperBase::ActivateBumper(APBBallBase* Ball)
{
	if (!IsValid(Ball) || !CanActivate())
	{
		return;
	}

	SetBumperState(EPBBumperState::Activated);

	OnBumperActivated(Ball);

	if (IsValid(BumperEffect))
	{
		BumperEffect->ActivateEffect(this, Ball);
		return;
	}

	ApplyBumperEffect(Ball);
}

void APBModularBumperBase::FinishActivation()
{
	if (CurrentState != EPBBumperState::Activated)
	{
		return;
	}

	SetBumperState(EPBBumperState::Idle);

	for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor))
		{
			TriggerActor->FinishTrigger();
		}
	}

	ResetTriggerCount();
	OnBumperFinished();
}

void APBModularBumperBase::ResetTriggerCount()
{
	CurrentTriggerCount = 0;
	NotifyTriggerCountChanged();
}

void APBModularBumperBase::SetBumperState(const EPBBumperState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EPBBumperState PreviousState = CurrentState;
	CurrentState = NewState;

	for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor))
		{
			TriggerActor->SetTriggerState(CurrentState);
		}
	}

	OnBumperStateChanged.Broadcast(PreviousState, CurrentState);
}

bool APBModularBumperBase::CanAccumulateTrigger() const
{
	return CurrentState == EPBBumperState::Idle;
}

bool APBModularBumperBase::CanActivate() const
{
	return CurrentState == EPBBumperState::Idle
		&& CurrentTriggerCount >= GetRequiredTriggerCount();
}

int32 APBModularBumperBase::GetCurrentTriggerCount() const
{
	return CurrentTriggerCount;
}

int32 APBModularBumperBase::GetRequiredTriggerCount() const
{
	return FMath::Max(BumperData.RequiredTriggerCount, 1);
}

EPBBumperState APBModularBumperBase::GetBumperState() const
{
	return CurrentState;
}

void APBModularBumperBase::CreateBumperEffect()
{
	if (!EffectClass)
	{
		return;
	}

	BumperEffect = NewObject<UPBBumperEffectBase>(this, EffectClass);
	if (IsValid(BumperEffect))
	{
		BumperEffect->Initialize(this);
	}
}

void APBModularBumperBase::AddTriggerCount(APBBallBase* Ball, const int32 Amount)
{
	if (!IsValid(Ball) || !CanAccumulateTrigger() || Amount <= 0)
	{
		return;
	}

	const int32 RequiredTriggerCount = GetRequiredTriggerCount();
	const int32 PreviousTriggerCount = CurrentTriggerCount;
	CurrentTriggerCount = FMath::Clamp(CurrentTriggerCount + Amount, 0, RequiredTriggerCount);

	NotifyTriggerCountChanged();

	//콤보 증가
	if (IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(Ball))
	{
		Comboable->AddCombo(Amount);
		//UE_LOG(LogTemp, Warning, TEXT("combo = %d"), Comboable->GetCombo());
	}

	// 이번 증가로 처음 조건을 만족했을 때만 Ready 이벤트를 보낸다.
	// TODO 준비는 따로 필요없을듯 하다. 즉시 시전되면 될듯.
	if (PreviousTriggerCount < RequiredTriggerCount && CurrentTriggerCount >= RequiredTriggerCount)
	{
		OnBumperReady();
	}

	if (CanActivate())
	{
		ActivateBumper(Ball);
	}
}

APBBumperTriggerActorBase* APBModularBumperBase::SpawnTriggerActor(
	TSubclassOf<APBBumperTriggerActorBase> TriggerClass,
	const EPBBumperPositionId PositionId)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !TriggerClass)
	{
		return nullptr;
	}

	FTransform SpawnTransform;
	if (!FindBumperPositionTransform(PositionId, SpawnTransform))
	{
		return nullptr;
	}

	APBBumperTriggerActorBase* TriggerActor = World->SpawnActorDeferred<APBBumperTriggerActorBase>(
		TriggerClass,
		SpawnTransform,
		this);

	if (!IsValid(TriggerActor))
	{
		return nullptr;
	}

	TriggerActor->InitializeTrigger(this);
	UGameplayStatics::FinishSpawningActor(TriggerActor, SpawnTransform);
	TriggerActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	TriggerActor->SetTriggerState(CurrentState);
	SpawnedTriggerActors.Add(TriggerActor);

	return TriggerActor;
}

void APBModularBumperBase::SpawnTriggerActors(
	TSubclassOf<APBBumperTriggerActorBase> TriggerClass,
	const TArray<EPBBumperPositionId>& PositionIds)
{
	for (const EPBBumperPositionId PositionId : PositionIds)
	{
		SpawnTriggerActor(TriggerClass, PositionId);
	}
}

void APBModularBumperBase::SpawnTriggerActorsFromInfo(const FPBBumperTriggerSpawnInfo& SpawnInfo)
{
	SpawnTriggerActors(SpawnInfo.TriggerClass, SpawnInfo.PositionIds);
}

void APBModularBumperBase::ClearTriggerActors()
{
	for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor))
		{
			TriggerActor->Destroy();
		}
	}

	SpawnedTriggerActors.Reset();
}

bool APBModularBumperBase::FindBumperPositionTransform(
	const EPBBumperPositionId PositionId,
	FTransform& OutTransform) const
{
	if (PositionId == EPBBumperPositionId::None)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	for (TActorIterator<APBBumperPositionAnchor> It(World); It; ++It)
	{
		const APBBumperPositionAnchor* Anchor = *It;
		if (!IsValid(Anchor) || Anchor->GetPositionId() != PositionId)
		{
			continue;
		}

		OutTransform = Anchor->GetActorTransform();
		return true;
	}

	return false;
}

void APBModularBumperBase::NotifyTriggerCountChanged()
{
	const int32 RequiredTriggerCount = GetRequiredTriggerCount();
	OnBumperTriggerCountChanged.Broadcast(CurrentTriggerCount, RequiredTriggerCount);
	OnTriggerCountChanged(CurrentTriggerCount, RequiredTriggerCount);
}

void APBModularBumperBase::ApplyBumperEffect_Implementation(APBBallBase* Ball)
{
	// 자식에서 효과를 구현하지 않으면 즉시 Idle로 돌려 테스트하기 쉽게 둔다.
	FinishActivation();
}
