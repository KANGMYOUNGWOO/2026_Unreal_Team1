// Fill out your copyright notice in the Description page of Project Settings.


#include "PBModularBumperBase.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBBumperPositionAnchor.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"

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

	for (const FPBBumperTriggerSpawnInfo& SpawnInfo : TriggerSpawnInfos)
	{
		SpawnTriggerActorsFromInfo(SpawnInfo);
	}

	NotifyTriggerCountChanged();
}

void APBModularBumperBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearTriggerActors();

	Super::EndPlay(EndPlayReason);
}

void APBModularBumperBase::HandleTriggerActorActivated(
	APBBumperTriggerActorBase* TriggerActor,
	AActor* InteractionActor,
	const FHitResult& TriggerHit)
{
	if (!IsValid(TriggerActor) || !IsValid(InteractionActor) || !CanAccumulateTrigger())
	{
		return;
	}

	AddTriggerCount(TriggerActor, InteractionActor);
}

void APBModularBumperBase::ActivateBumper(AActor* InteractionActor)
{
	if (!IsValid(InteractionActor) || !CanActivate())
	{
		return;
	}

	if (APBBumperTriggerActorBase* ReadyTrigger = FindReadyTrigger())
	{
		StartActivation(ReadyTrigger, InteractionActor);
	}
}

void APBModularBumperBase::FinishActivation()
{
	APBBumperTriggerActorBase* FinishedTrigger = ActiveTriggerActor.Get();
	if (!IsValid(FinishedTrigger) && RuntimeState.CurrentState != EPBBumperState::Activated)
	{
		return;
	}

	ActiveTriggerActor.Reset();
	if (IsValid(FinishedTrigger))
	{
		FinishedTrigger->FinishTrigger();
	}

	// 전투 흐름이 범퍼를 Disabled로 바꾼 경우, 효과 종료가 그 상태를 임의로 해제하지 않는다.
	if (RuntimeState.CurrentState != EPBBumperState::Disabled)
	{
		SetBumperState(EPBBumperState::Idle);
	}

	NotifyTriggerCountChanged();
	OnBumperFinished();
	ScheduleNextPendingActivation();
}

void APBModularBumperBase::ResetTriggerCount()
{
	PendingActivations.Reset();

	for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor) && TriggerActor != ActiveTriggerActor.Get())
		{
			TriggerActor->ResetTriggerProgress();
		}
	}

	NotifyTriggerCountChanged();
}

void APBModularBumperBase::SetBumperState(const EPBBumperState NewState)
{
	if (NewState == EPBBumperState::Activated && !ActiveTriggerActor.IsValid())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Ignored Activated state without an active Trigger. Bumper=%s"),
			*GetNameSafe(this));
		return;
	}

	if (RuntimeState.CurrentState == NewState)
	{
		return;
	}

	const EPBBumperState PreviousState = RuntimeState.CurrentState;
	RuntimeState.CurrentState = NewState;

	if (NewState == EPBBumperState::Disabled || PreviousState == EPBBumperState::Disabled)
	{
		for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
		{
			if (!IsValid(TriggerActor))
			{
				continue;
			}

			if (NewState == EPBBumperState::Disabled)
			{
				TriggerActor->SetTriggerState(EPBBumperState::Disabled);
			}
			else
			{
				TriggerActor->SetTriggerState(
					TriggerActor == ActiveTriggerActor.Get()
						? EPBBumperState::Activated
						: EPBBumperState::Idle);
			}
		}
	}

	OnBumperStateChanged.Broadcast(PreviousState, RuntimeState.CurrentState);

	if (NewState == EPBBumperState::Idle && !ActiveTriggerActor.IsValid())
	{
		ScheduleNextPendingActivation();
	}
}

bool APBModularBumperBase::CanAccumulateTrigger() const
{
	return RuntimeState.CurrentState != EPBBumperState::Disabled;
}

bool APBModularBumperBase::CanActivate() const
{
	return RuntimeState.CurrentState == EPBBumperState::Idle
		&& !ActiveTriggerActor.IsValid()
		&& IsValid(FindReadyTrigger());
}

int32 APBModularBumperBase::GetCurrentTriggerCount() const
{
	int32 HighestTriggerCount = 0;
	for (const APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor))
		{
			HighestTriggerCount = FMath::Max(
				HighestTriggerCount,
				TriggerActor->GetCurrentTriggerCount());
		}
	}

	return HighestTriggerCount;
}

int32 APBModularBumperBase::GetRequiredTriggerCount() const
{
	return FMath::Max(BumperData.RequiredTriggerCount, 1);
}

EPBBumperState APBModularBumperBase::GetBumperState() const
{
	return RuntimeState.CurrentState;
}

APBBumperTriggerActorBase* APBModularBumperBase::GetActiveTriggerActor() const
{
	return ActiveTriggerActor.Get();
}

int32 APBModularBumperBase::GetPendingActivationCount() const
{
	return PendingActivations.Num();
}

void APBModularBumperBase::CreateBumperEffect()
{
	if (!EffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Cannot create effect. Bumper=%s EffectId=%s"),
			*GetNameSafe(this),
			*BumperData.EffectID.ToString());
		return;
	}

	BumperEffect = NewObject<UPBBumperEffectBase>(this, EffectClass);
	if (!IsValid(BumperEffect))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Failed to instantiate effect. Bumper=%s EffectClass=%s"),
			*GetNameSafe(this),
			*GetNameSafe(EffectClass));
		return;
	}

	BumperEffect->InitializeEffect(this, EffectData);
	UE_LOG(LogTemp, Log, TEXT("[Bumper] Effect ready. Bumper=%s EffectId=%s EffectClass=%s Power=%.2f"),
		*GetNameSafe(this),
		*BumperData.EffectID.ToString(),
		*GetNameSafe(EffectClass),
		EffectData.Power);
}

void APBModularBumperBase::InitializeBumper(
	const FPBBumperTableRow& InBumperData,
	const TArray<FPBBumperTriggerSpawnInfo>& InTriggerSpawnInfos,
	const FPBBumperEffectRow& InEffectData,
	TSubclassOf<UPBBumperEffectBase> InEffectClass,
	const TMap<EPBBumperPositionId, FTransform>& InAnchorTransforms)
{
	BumperData = InBumperData;
	TriggerSpawnInfos = InTriggerSpawnInfos;
	EffectData = InEffectData;
	EffectClass = InEffectClass;
	AnchorTransforms = InAnchorTransforms;
}

void APBModularBumperBase::AddTriggerCount(
	APBBumperTriggerActorBase* TriggerActor,
	AActor* InteractionActor,
	const int32 Amount)
{
	if (!IsValid(TriggerActor)
		|| TriggerActor->GetOwnerBumper() != this
		|| !IsValid(InteractionActor)
		|| !CanAccumulateTrigger()
		|| Amount <= 0)
	{
		return;
	}

	const bool bBecameReady = TriggerActor->AddTriggerProgress(Amount);
	NotifyTriggerCountChanged();

	UE_LOG(LogTemp, Verbose,
		TEXT("[Bumper] Trigger progress. Bumper=%s Trigger=%s Position=%d Count=%d/%d"),
		*GetNameSafe(this),
		*GetNameSafe(TriggerActor),
		static_cast<int32>(TriggerActor->GetPositionId()),
		TriggerActor->GetCurrentTriggerCount(),
		TriggerActor->GetRequiredTriggerCount());

	if (bBecameReady)
	{
		OnBumperReady();
		RequestActivation(TriggerActor, InteractionActor);
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

	TriggerActor->InitializeTrigger(this, PositionId, GetRequiredTriggerCount());
	UGameplayStatics::FinishSpawningActor(TriggerActor, SpawnTransform);
	TriggerActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	TriggerActor->SetTriggerState(
		RuntimeState.CurrentState == EPBBumperState::Disabled
			? EPBBumperState::Disabled
			: EPBBumperState::Idle);
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
	PendingActivations.Reset();
	ActiveTriggerActor.Reset();
	bPendingActivationScheduled = false;

	for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor))
		{
			TriggerActor->Destroy();
		}
	}

	SpawnedTriggerActors.Reset();
	RuntimeState.CurrentTriggerCount = 0;
}

APBBumperTriggerActorBase* APBModularBumperBase::FindReadyTrigger() const
{
	for (APBBumperTriggerActorBase* TriggerActor : SpawnedTriggerActors)
	{
		if (IsValid(TriggerActor)
			&& TriggerActor->GetTriggerProgressState() == EPBBumperTriggerProgressState::Ready)
		{
			return TriggerActor;
		}
	}

	return nullptr;
}

void APBModularBumperBase::RequestActivation(
	APBBumperTriggerActorBase* TriggerActor,
	AActor* InteractionActor)
{
	if (!IsValid(TriggerActor)
		|| !IsValid(InteractionActor)
		|| !TriggerActor->IsTriggerReady())
	{
		return;
	}

	if (RuntimeState.CurrentState == EPBBumperState::Idle && !ActiveTriggerActor.IsValid())
	{
		StartActivation(TriggerActor, InteractionActor);
		return;
	}

	QueueActivation(TriggerActor, InteractionActor);
}

void APBModularBumperBase::QueueActivation(
	APBBumperTriggerActorBase* TriggerActor,
	AActor* InteractionActor)
{
	if (!IsValid(TriggerActor)
		|| !IsValid(InteractionActor)
		|| !TriggerActor->IsTriggerReady()
		|| HasPendingActivationFor(TriggerActor))
	{
		return;
	}

	TriggerActor->SetTriggerProgressState(EPBBumperTriggerProgressState::Queued);
	PendingActivations.Add({TriggerActor, InteractionActor});

	if (EffectData.ExecutionPolicy == EPBBumperEffectExecutionPolicy::Immediate)
	{
		// Immediate 효과가 이 경로에 들어오면 이전 효과가 FinishEffect를 아직 호출하지 않은 것이다.
		// 같은 Effect UObject 재진입을 막기 위해 안전하게 직렬화하고 설정 오류를 로그로 드러낸다.
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Immediate effect request was serialized because the execution lane is busy. Bumper=%s Trigger=%s Pending=%d"),
			*GetNameSafe(this),
			*GetNameSafe(TriggerActor),
			PendingActivations.Num());
	}
	else
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Effect request queued. Bumper=%s Trigger=%s Pending=%d"),
			*GetNameSafe(this),
			*GetNameSafe(TriggerActor),
			PendingActivations.Num());
	}
}

void APBModularBumperBase::StartActivation(
	APBBumperTriggerActorBase* TriggerActor,
	AActor* InteractionActor)
{
	if (!IsValid(TriggerActor)
		|| !IsValid(InteractionActor)
		|| !TriggerActor->IsTriggerReady())
	{
		return;
	}

	if (RuntimeState.CurrentState != EPBBumperState::Idle || ActiveTriggerActor.IsValid())
	{
		QueueActivation(TriggerActor, InteractionActor);
		return;
	}

	ActiveTriggerActor = TriggerActor;
	TriggerActor->SetTriggerProgressState(EPBBumperTriggerProgressState::Executing);
	TriggerActor->SetTriggerState(EPBBumperState::Activated);
	ExecuteActivation(InteractionActor);
}

void APBModularBumperBase::ExecuteActivation(AActor* InteractionActor)
{
	if (!IsValid(InteractionActor) || !ActiveTriggerActor.IsValid())
	{
		if (ActiveTriggerActor.IsValid())
		{
			FinishActivation();
		}
		return;
	}

	SetBumperState(EPBBumperState::Activated);
	OnMovableActorActivated(InteractionActor);
	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		OnBumperActivated(Ball);
	}
	if (!ActiveTriggerActor.IsValid())
	{
		return;
	}

	if (IsValid(BumperEffect))
	{
		BumperEffect->ActivateEffectForActor(this, InteractionActor);
		return;
	}

	ApplyBumperEffectToActor(InteractionActor);
}

void APBModularBumperBase::ScheduleNextPendingActivation()
{
	if (PendingActivations.IsEmpty()
		|| ActiveTriggerActor.IsValid()
		|| RuntimeState.CurrentState == EPBBumperState::Disabled
		|| bPendingActivationScheduled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	bPendingActivationScheduled = true;
	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ProcessNextPendingActivation();
		}));
}

void APBModularBumperBase::ProcessNextPendingActivation()
{
	bPendingActivationScheduled = false;
	if (ActiveTriggerActor.IsValid()
		|| RuntimeState.CurrentState != EPBBumperState::Idle)
	{
		return;
	}

	while (!PendingActivations.IsEmpty())
	{
		const FPendingBumperActivation PendingActivation = MoveTemp(PendingActivations[0]);
		PendingActivations.RemoveAt(0, 1, EAllowShrinking::No);

		APBBumperTriggerActorBase* TriggerActor = PendingActivation.TriggerActor.Get();
		if (!IsValid(TriggerActor) || !TriggerActor->IsTriggerReady())
		{
			// 대기 중 Trigger가 파괴되거나 외부 초기화된 경우 모듈 호환 UI 값도 다시 계산한다.
			NotifyTriggerCountChanged();
			continue;
		}

		AActor* InteractionActor = PendingActivation.InteractionActor.Get();
		if (!IsValid(InteractionActor))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[Bumper] Queued effect request discarded because the interaction Actor is invalid. Bumper=%s Trigger=%s"),
				*GetNameSafe(this),
				*GetNameSafe(TriggerActor));
			TriggerActor->FinishTrigger();
			NotifyTriggerCountChanged();
			continue;
		}

		StartActivation(TriggerActor, InteractionActor);
		return;
	}
}

bool APBModularBumperBase::HasPendingActivationFor(
	const APBBumperTriggerActorBase* TriggerActor) const
{
	return PendingActivations.ContainsByPredicate(
		[TriggerActor](const FPendingBumperActivation& PendingActivation)
		{
			return PendingActivation.TriggerActor.Get() == TriggerActor;
		});
}

bool APBModularBumperBase::FindBumperPositionTransform(
	const EPBBumperPositionId PositionId,
	FTransform& OutTransform) const
{
	if (PositionId == EPBBumperPositionId::None)
	{
		return false;
	}

	if (const FTransform* AnchorTransform = AnchorTransforms.Find(PositionId))
	{
		OutTransform = *AnchorTransform;
		return true;
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
	// 기존 모듈 단위 UI가 즉시 깨지지 않도록 자식 Trigger 중 가장 높은 진행도를 호환값으로 제공한다.
	RuntimeState.CurrentTriggerCount = GetCurrentTriggerCount();
	OnBumperTriggerCountChanged.Broadcast(RuntimeState.CurrentTriggerCount, RequiredTriggerCount);
	OnTriggerCountChanged(RuntimeState.CurrentTriggerCount, RequiredTriggerCount);
}

void APBModularBumperBase::ApplyBumperEffectToActor_Implementation(AActor* InteractionActor)
{
	// 기존 Ball Blueprint 구현은 그대로 재사용하고, 일반 Actor에 구현이 없으면 실행 레인을 해제한다.
	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		ApplyBumperEffect(Ball);
		return;
	}

	FinishActivation();
}

void APBModularBumperBase::ApplyBumperEffect_Implementation(APBBallBase* Ball)
{
	// 자식에서 효과를 구현하지 않으면 즉시 Idle로 돌려 테스트하기 쉽게 둔다.
	FinishActivation();
}
