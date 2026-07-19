// Fill out your copyright notice in the Description page of Project Settings.


#include "PBModularBumperBase.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
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
	if (IsValid(BumperEffect))
	{
		BumperEffect->ShutdownEffect();
		BumperEffect = nullptr;
	}

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

EPBBumperPositionId APBModularBumperBase::GetPrimaryPositionId() const
{
	for (const FPBBumperTriggerSpawnInfo& SpawnInfo : TriggerSpawnInfos)
	{
		for (const EPBBumperPositionId PositionId : SpawnInfo.PositionIds)
		{
			if (PositionId != EPBBumperPositionId::None)
			{
				return PositionId;
			}
		}
	}

	return EPBBumperPositionId::None;
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
	const FName InBumperRowId,
	const FPBBumperTableRow& InBumperData,
	const TArray<FPBBumperTriggerSpawnInfo>& InTriggerSpawnInfos,
	const FPBBumperEffectRow& InEffectData,
	TSubclassOf<UPBBumperEffectBase> InEffectClass,
	UNiagaraSystem* InActivationVfx,
	UNiagaraSystem* InDeliveryVfx,
	UNiagaraSystem* InImpactVfx,
	UNiagaraSystem* InStatusVfx,
	const TMap<EPBBumperPositionId, FTransform>& InAnchorTransforms)
{
	BumperRowId = InBumperRowId;
	BumperData = InBumperData;
	RuntimeState.MeaningfulContactCount = 0;
	RuntimeState.ActivationCount = 0;
	TriggerSpawnInfos = InTriggerSpawnInfos;
	EffectData = InEffectData;
	EffectClass = InEffectClass;
	ActivationVfx = InActivationVfx;
	DeliveryVfx = InDeliveryVfx;
	ImpactVfx = InImpactVfx;
	StatusVfx = InStatusVfx;
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

	const int32 PreviousTriggerCount = TriggerActor->GetCurrentTriggerCount();
	const bool bBecameReady = TriggerActor->AddTriggerProgress(Amount);
	if (TriggerActor->GetCurrentTriggerCount() > PreviousTriggerCount)
	{
		++RuntimeState.MeaningfulContactCount;
		UE_LOG(LogTemp, Log,
			TEXT("[BumperTelemetry] MeaningfulContact RowName=%s Position=%s Contacts=%d Progress=%d/%d"),
			*BumperRowId.ToString(),
			*UEnum::GetValueAsString(TriggerActor->GetPositionId()),
			RuntimeState.MeaningfulContactCount,
			TriggerActor->GetCurrentTriggerCount(),
			TriggerActor->GetRequiredTriggerCount());
	}
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

	const EPBBumperPositionId ActivationPositionId = ActiveTriggerActor->GetPositionId();
	SetBumperState(EPBBumperState::Activated);
	++RuntimeState.ActivationCount;
	UE_LOG(LogTemp, Log,
		TEXT("[BumperTelemetry] Activation RowName=%s Position=%s Activations=%d Contacts=%d"),
		*BumperRowId.ToString(),
		*UEnum::GetValueAsString(ActivationPositionId),
		RuntimeState.ActivationCount,
		RuntimeState.MeaningfulContactCount);
	SpawnActivationVfx();
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

void APBModularBumperBase::SpawnActivationVfx() const
{
	if (!IsValid(ActivationVfx))
	{
		return;
	}

	const AActor* SpawnSource = ActiveTriggerActor.IsValid()
		? static_cast<const AActor*>(ActiveTriggerActor.Get())
		: static_cast<const AActor*>(this);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		ActivationVfx.Get(),
		SpawnSource->GetActorLocation(),
		SpawnSource->GetActorRotation(),
		FVector::OneVector,
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
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
	RuntimeState.CurrentTriggerCount = GetCurrentTriggerCount();
	OnBumperTriggerCountChanged.Broadcast(RuntimeState.CurrentTriggerCount, RequiredTriggerCount);
	OnTriggerCountChanged(RuntimeState.CurrentTriggerCount, RequiredTriggerCount);
}

void APBModularBumperBase::ApplyBumperEffectToActor_Implementation(AActor* InteractionActor)
{
	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		ApplyBumperEffect(Ball);
		return;
	}

	FinishActivation();
}

void APBModularBumperBase::ApplyBumperEffect_Implementation(APBBallBase* Ball)
{
	FinishActivation();
}
