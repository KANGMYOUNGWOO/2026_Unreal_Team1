// Fill out your copyright notice in the Description page of Project Settings.


#include "PBSummonBumperEffect.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"

EPBBumperSummonAnchorType UPBSummonBumperEffect::GetSpawnAnchorType() const
{
	return SpawnAnchorType;
}

void UPBSummonBumperEffect::Initialize(APBModularBumperBase* InOwnerBumper)
{
	Super::Initialize(InOwnerBumper);

	EnsureSummonActor(InOwnerBumper);
}

void UPBSummonBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	APBModularBumperBase* TargetBumper = IsValid(Bumper) ? Bumper : OwnerBumper.Get();
	if (!IsValid(TargetBumper)
		|| !IsValid(InteractionActor)
		|| !EnsureSummonActor(TargetBumper))
	{
		FinishEffect();
		return;
	}

	SpawnedSummonActor->StartActionForActor(TargetBumper, InteractionActor);
}

void UPBSummonBumperEffect::FinishEffect()
{
	if (IsDestroyOnFinished)
	{
		DestroySummonActor();
	}
	else
	{
		DeactivateSummonActor();
	}

	Super::FinishEffect();
}

void UPBSummonBumperEffect::ShutdownEffect()
{
	DestroySummonActor();
	Super::ShutdownEffect();
}

void UPBSummonBumperEffect::HandleSummonActionFinished(APBBumperSummonActor* SummonActor)
{
	if (SummonActor != SpawnedSummonActor)
	{
		return;
	}

	StopStatusVfx(SummonActor);
	FinishEffect();
}

bool UPBSummonBumperEffect::EnsureSummonActor(APBModularBumperBase* Bumper)
{
	if (!IsValid(Bumper) || !SummonActorClass)
	{
		return false;
	}

	UWorld* World = Bumper->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	bool bUsesSummonAnchor = false;
	const FTransform SpawnTransform = ResolveSpawnTransform(Bumper, bUsesSummonAnchor);
	if (IsValid(SpawnedSummonActor))
	{
		UpdateSummonActorTransform(Bumper, SpawnTransform, bUsesSummonAnchor);
		return true;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Bumper;

	// 범퍼가 생성될 때 소환 객체를 미리 배치해두고, 발동 시에는 동작만 실행한다.
	SpawnedSummonActor = World->SpawnActor<APBBumperSummonActor>(
		SummonActorClass,
		SpawnTransform,
		SpawnParameters);

	if (!IsValid(SpawnedSummonActor))
	{
		SpawnedSummonActor = nullptr;
		return false;
	}

	UpdateSummonActorTransform(Bumper, SpawnTransform, bUsesSummonAnchor);
	SpawnedSummonActor->OnSummonActionFinished.AddUniqueDynamic(
		this,
		&UPBSummonBumperEffect::HandleSummonActionFinished);

	return true;
}

FTransform UPBSummonBumperEffect::ResolveSpawnTransform(
	APBModularBumperBase* Bumper,
	bool& bOutUsesSummonAnchor)
{
	bOutUsesSummonAnchor = false;
	const FTransform FallbackTransform = SpawnOffset * Bumper->GetActorTransform();
	if (SpawnAnchorType == EPBBumperSummonAnchorType::None)
	{
		return FallbackTransform;
	}

	UWorld* World = Bumper->GetWorld();
	if (!IsValid(World))
	{
		return FallbackTransform;
	}

	const EPBBumperPositionId SourcePositionId = ResolveSourcePositionId(Bumper);
	APBBumperSummonAnchor* ExactAnchor = nullptr;
	APBBumperSummonAnchor* SharedAnchor = nullptr;
	int32 ExactAnchorCount = 0;
	int32 SharedAnchorCount = 0;

	for (TActorIterator<APBBumperSummonAnchor> It(World); It; ++It)
	{
		APBBumperSummonAnchor* Anchor = *It;
		if (!IsValid(Anchor) || Anchor->GetAnchorType() != SpawnAnchorType)
		{
			continue;
		}

		const EPBBumperPositionId AnchorPositionId = Anchor->GetSourcePositionId();
		if (SourcePositionId != EPBBumperPositionId::None
			&& AnchorPositionId == SourcePositionId)
		{
			ExactAnchor = Anchor;
			++ExactAnchorCount;
		}
		else if (AnchorPositionId == EPBBumperPositionId::None)
		{
			SharedAnchor = Anchor;
			++SharedAnchorCount;
		}
	}

	APBBumperSummonAnchor* ResolvedAnchor = nullptr;
	if (ExactAnchorCount == 1)
	{
		ResolvedAnchor = ExactAnchor;
	}
	else if (ExactAnchorCount == 0 && SharedAnchorCount == 1)
	{
		ResolvedAnchor = SharedAnchor;
	}

	if (IsValid(ResolvedAnchor))
	{
		bHasReportedAnchorResolutionFailure = false;
		bOutUsesSummonAnchor = true;
		return SpawnOffset * ResolvedAnchor->GetActorTransform();
	}

	if (!bHasReportedAnchorResolutionFailure)
	{
		const TCHAR* FailureReason = ExactAnchorCount > 1
			? TEXT("duplicate position-specific Anchors")
			: SharedAnchorCount > 1
				? TEXT("duplicate shared Anchors")
				: TEXT("no matching Anchor");
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Summon Anchor resolution failed; using the Bumper-relative fallback. Bumper=%s AnchorType=%d SourcePosition=%d Reason=%s"),
			*GetNameSafe(Bumper),
			static_cast<int32>(SpawnAnchorType),
			static_cast<int32>(SourcePositionId),
			FailureReason);
		bHasReportedAnchorResolutionFailure = true;
	}

	return FallbackTransform;
}

EPBBumperPositionId UPBSummonBumperEffect::ResolveSourcePositionId(
	const APBModularBumperBase* Bumper) const
{
	if (!IsValid(Bumper))
	{
		return EPBBumperPositionId::None;
	}

	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	if (IsValid(ActiveTrigger)
		&& ActiveTrigger->GetPositionId() != EPBBumperPositionId::None)
	{
		return ActiveTrigger->GetPositionId();
	}

	return Bumper->GetPrimaryPositionId();
}

void UPBSummonBumperEffect::UpdateSummonActorTransform(
	APBModularBumperBase* Bumper,
	const FTransform& SpawnTransform,
	const bool bUsesSummonAnchor) const
{
	if (!IsValid(SpawnedSummonActor) || !IsValid(Bumper))
	{
		return;
	}

	if (bUsesSummonAnchor)
	{
		SpawnedSummonActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	else if (SpawnedSummonActor->GetAttachParentActor() != Bumper)
	{
		SpawnedSummonActor->AttachToActor(Bumper, FAttachmentTransformRules::KeepWorldTransform);
	}

	SpawnedSummonActor->SetActorTransform(SpawnTransform);
}

void UPBSummonBumperEffect::DeactivateSummonActor() const
{
	if (IsValid(SpawnedSummonActor))
	{
		StopStatusVfx(SpawnedSummonActor);
		SpawnedSummonActor->DeactivateSummon();
	}
}

void UPBSummonBumperEffect::DestroySummonActor()
{
	if (!IsValid(SpawnedSummonActor))
	{
		SpawnedSummonActor = nullptr;
		return;
	}

	SpawnedSummonActor->OnSummonActionFinished.RemoveDynamic(
		this,
		&UPBSummonBumperEffect::HandleSummonActionFinished);
	SpawnedSummonActor->DeactivateSummon();
	SpawnedSummonActor->Destroy();

	SpawnedSummonActor = nullptr;
}
