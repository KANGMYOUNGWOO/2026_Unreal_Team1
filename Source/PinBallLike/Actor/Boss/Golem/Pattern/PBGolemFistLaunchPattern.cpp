#include "PBGolemFistLaunchPattern.h"

#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossHand.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemHandMovementComponent.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossChargeTelegraph.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternTelegraph.h"

bool UPBGolemFistLaunchPattern::UsesHand(EPBGolemBossHandType TargetHandType) const
{
	return HandType == TargetHandType;
}

bool UPBGolemFistLaunchPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	const APBGolemBoss* GolemBoss = Cast<APBGolemBoss>(Boss);
	const APBGolemBossHand* GolemHand = GolemBoss ? GolemBoss->GetGolemHand(HandType) : nullptr;
	const bool IsCanExecute = Super::CanExecute_Implementation(Boss)
		&& GolemBoss
		&& GolemHand
		&& GolemHand->IsHandAvailable();

	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] CanExecute. Pattern=%s Boss=%s HandType=%d Success=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType),
		IsCanExecute ? TEXT("true") : TEXT("false"));

	return IsCanExecute;
}

void UPBGolemFistLaunchPattern::StartPattern_Implementation(APBBossBase* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] StartPattern. Pattern=%s Boss=%s HandType=%d"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType));

	if (!Boss)
	{
		FinishFistLaunchPattern();
		return;
	}

	SetOwnerBoss(Boss);
	if (APBGolemBoss* GolemBoss = GetGolemBoss(Boss))
	{
		if (APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType))
		{
			GolemHand->StopAutonomousMove();
			StartHandTransform = GolemHand->GetActorTransform();
		}
	}

	StartAiming(Boss);
}

void UPBGolemFistLaunchPattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] ExecutePattern started. Pattern=%s Boss=%s HandType=%d"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType));

	APBGolemBoss* GolemBoss = GetGolemBoss(Boss);
	if (FistTargetLocation.IsNearlyZero())
	{
		if (AActor* PinballActor = FindPinballActor())
		{
			FistTargetLocation = PinballActor->GetActorLocation();
		}
	}

	if (!GolemBoss || FistTargetLocation.IsNearlyZero() || !TryBindHandMoveFinished())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemFistLaunchPattern] ExecutePattern failed. GolemBoss=%s TargetLocation=%s BoundMovement=%s"),
			*GetNameSafe(GolemBoss),
			*FistTargetLocation.ToString(),
			BoundMovementComponent ? TEXT("true") : TEXT("false"));
		FinishFistLaunchPattern();
		return;
	}

	FistLaunchPhase = EPBGolemFistLaunchPhase::Launching;
	APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType);
	if (!GolemHand)
	{
		FinishFistLaunchPattern();
		return;
	}

	GolemHand->BeginPatternMovementLock();
	GolemHand->LaunchFistAtLocationForPattern(FistTargetLocation, FistTargetDirection, LaunchDuration);
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] ExecutePattern succeeded. HandType=%d TargetLocation=%s Direction=%s LaunchDuration=%.2f"),
		static_cast<int32>(HandType),
		*FistTargetLocation.ToString(),
		*FistTargetDirection.ToString(),
		LaunchDuration);
}

void UPBGolemFistLaunchPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] CancelPatternInternal. Pattern=%s Boss=%s HandType=%d"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType));

	UnbindHandMoveFinished();
	ClearFistTelegraphs();
	FistLaunchPhase = EPBGolemFistLaunchPhase::None;

	if (APBGolemBoss* GolemBoss = GetGolemBoss(Boss))
	{
		ReturnHandToStartTransform(GolemBoss);
		UnlockPatternHand();
		UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] CancelPatternInternal return requested. HandType=%d ReturnDuration=%.2f"),
			static_cast<int32>(HandType),
			ReturnDuration);
	}
}

void UPBGolemFistLaunchPattern::StartAiming(APBBossBase* Boss)
{
	SpawnFistTelegraphs(Boss);

	const float TelegraphDurationSeconds = GetFistTelegraphDurationSeconds();
	if (TelegraphDurationSeconds <= 0.0f || SpawnedFistChargeTelegraphs.IsEmpty())
	{
		HandleFistTelegraphFinished(FVector::ZeroVector, FVector::ForwardVector);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] StartAiming succeeded. TelegraphDuration=%.2f"),
		TelegraphDurationSeconds);
}

float UPBGolemFistLaunchPattern::GetFistTelegraphDurationSeconds() const
{
	return TelegraphDataList.IsEmpty() ? 0.0f : TelegraphDataList[0].DurationSeconds;
}

void UPBGolemFistLaunchPattern::SpawnFistTelegraphs(APBBossBase* Boss)
{
	ClearFistTelegraphs();

	if (!Boss || TelegraphDataList.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] SpawnFistTelegraphs skipped. Boss=%s HasTelegraphData=%s"),
			*GetNameSafe(Boss),
			TelegraphDataList.IsEmpty() ? TEXT("false") : TEXT("true"));
		return;
	}

	const TArray<APBBossPatternTelegraph*> SpawnedTelegraphList = SpawnTelegraph(Boss);
	for (APBBossPatternTelegraph* SpawnedTelegraph : SpawnedTelegraphList)
	{
		APBBossChargeTelegraph* ChargeTelegraph = Cast<APBBossChargeTelegraph>(SpawnedTelegraph);
		if (!ChargeTelegraph)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GolemFistLaunchPattern] SpawnFistTelegraphs ignored non-charge telegraph. Telegraph=%s"),
				*GetNameSafe(SpawnedTelegraph));
			continue;
		}

		ChargeTelegraph->OnChargeTelegraphFinished.AddUniqueDynamic(
			this,
			&UPBGolemFistLaunchPattern::HandleFistTelegraphFinished);

		APBGolemBossHand* GolemHand = GetGolemHand(HandType);
		USceneComponent* TelegraphStartComponent = GolemHand ? GolemHand->GetTelegraphStartComponent() : nullptr;
		UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] AssignChargeStartComponent. Pattern=%s HandType=%d Hand=%s HandLocation=%s StartComponent=%s StartComponentLocation=%s Telegraph=%s TelegraphLocation=%s"),
			*GetNameSafe(this),
			static_cast<int32>(HandType),
			*GetNameSafe(GolemHand),
			GolemHand ? *GolemHand->GetActorLocation().ToString() : TEXT("None"),
			*GetNameSafe(TelegraphStartComponent),
			TelegraphStartComponent ? *TelegraphStartComponent->GetComponentLocation().ToString() : TEXT("None"),
			*GetNameSafe(ChargeTelegraph),
			*ChargeTelegraph->GetActorLocation().ToString());
		ChargeTelegraph->SetChargeStartComponent(TelegraphStartComponent);
		SpawnedFistChargeTelegraphs.Add(ChargeTelegraph);
	}

	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] SpawnFistTelegraphs. Count=%d Success=%s"),
		SpawnedFistChargeTelegraphs.Num(),
		SpawnedFistChargeTelegraphs.Num() > 0 ? TEXT("true") : TEXT("false"));
}

void UPBGolemFistLaunchPattern::HandleFistTelegraphFinished(FVector TargetLocation, FVector Direction)
{
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] HandleFistTelegraphFinished. Pattern=%s TargetLocation=%s Direction=%s"),
		*GetNameSafe(this),
		*TargetLocation.ToString(),
		*Direction.ToString());

	UnbindFistTelegraphs();
	ClearFistTelegraphs();
	FistTargetLocation = TargetLocation;
	FistTargetDirection = Direction;

	if (APBBossBase* Boss = GetOwnerBoss())
	{
		ExecutePattern_Implementation(Boss);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GolemFistLaunchPattern] HandleFistTelegraphFinished failed. OwnerBoss is invalid."));
	FinishFistLaunchPattern();
}

void UPBGolemFistLaunchPattern::UnbindFistTelegraphs()
{
	for (APBBossChargeTelegraph* ChargeTelegraph : SpawnedFistChargeTelegraphs)
	{
		if (!IsValid(ChargeTelegraph))
		{
			continue;
		}

		ChargeTelegraph->OnChargeTelegraphFinished.RemoveDynamic(
			this,
			&UPBGolemFistLaunchPattern::HandleFistTelegraphFinished);
	}
}

void UPBGolemFistLaunchPattern::ClearFistTelegraphs()
{
	UnbindFistTelegraphs();
	DestroySpawnedTelegraphs();
	SpawnedFistChargeTelegraphs.Reset();
}

void UPBGolemFistLaunchPattern::HandleHandMoveFinished()
{
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] HandleHandMoveFinished. Pattern=%s Phase=%d"),
		*GetNameSafe(this),
		static_cast<int32>(FistLaunchPhase));

	APBGolemBoss* GolemBoss = GetGolemBoss();
	if (!GolemBoss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemFistLaunchPattern] HandleHandMoveFinished failed. GolemBoss is invalid."));
		FinishFistLaunchPattern();
		return;
	}

	switch (FistLaunchPhase)
	{
	case EPBGolemFistLaunchPhase::Launching:
		FistLaunchPhase = EPBGolemFistLaunchPhase::Returning;
		ReturnHandToStartTransform(GolemBoss);
		UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] Launch phase finished. Return requested. HandType=%d ReturnDuration=%.2f"),
			static_cast<int32>(HandType),
			ReturnDuration);
		break;
	case EPBGolemFistLaunchPhase::Returning:
	default:
		UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] Return phase finished. Pattern complete."));
		FinishFistLaunchPattern();
		break;
	}
}

bool UPBGolemFistLaunchPattern::TryBindHandMoveFinished()
{
	APBGolemBossHand* GolemHand = GetGolemHand(HandType);
	if (!GolemHand)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemFistLaunchPattern] TryBindHandMoveFinished failed. Hand is invalid. HandType=%d"),
			static_cast<int32>(HandType));
		return false;
	}

	BoundMovementComponent = GolemHand->GetHandMovementComponent();
	if (!BoundMovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemFistLaunchPattern] TryBindHandMoveFinished failed. MovementComponent is invalid. Hand=%s"),
			*GetNameSafe(GolemHand));
		return false;
	}

	BoundMovementComponent->OnMoveFinished.AddUniqueDynamic(this, &UPBGolemFistLaunchPattern::HandleHandMoveFinished);
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] TryBindHandMoveFinished succeeded. Hand=%s MovementComponent=%s"),
		*GetNameSafe(GolemHand),
		*GetNameSafe(BoundMovementComponent));
	return true;
}

void UPBGolemFistLaunchPattern::UnbindHandMoveFinished()
{
	if (BoundMovementComponent)
	{
		BoundMovementComponent->OnMoveFinished.RemoveDynamic(this, &UPBGolemFistLaunchPattern::HandleHandMoveFinished);
		UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] UnbindHandMoveFinished. MovementComponent=%s"),
			*GetNameSafe(BoundMovementComponent));
	}

	BoundMovementComponent = nullptr;
}

void UPBGolemFistLaunchPattern::ReturnHandToStartTransform(APBGolemBoss* GolemBoss)
{
	if (!GolemBoss)
	{
		return;
	}

	APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType);
	if (!GolemHand)
	{
		return;
	}

	GolemHand->MoveToWorldTransformForPattern(StartHandTransform, ReturnDuration);
}

void UPBGolemFistLaunchPattern::UnlockPatternHand()
{
	if (APBGolemBoss* GolemBoss = GetGolemBoss())
	{
		if (APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType))
		{
			GolemHand->EndPatternMovementLock();
		}
	}
}

void UPBGolemFistLaunchPattern::FinishFistLaunchPattern()
{
	UE_LOG(LogTemp, Log, TEXT("[GolemFistLaunchPattern] FinishFistLaunchPattern. Pattern=%s"),
		*GetNameSafe(this));

	ClearFistTelegraphs();
	UnbindHandMoveFinished();
	UnlockPatternHand();
	FistTargetLocation = FVector::ZeroVector;
	FistTargetDirection = FVector::ForwardVector;
	StartHandTransform = FTransform::Identity;
	FistLaunchPhase = EPBGolemFistLaunchPhase::None;
	FinishPattern();
}
