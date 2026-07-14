#include "PBGolemHandSlamPattern.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "DrawDebugHelpers.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossHand.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemHandMovementComponent.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

UPBGolemHandSlamPattern::UPBGolemHandSlamPattern()
{
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> SlamEffectFinder(
		TEXT("/Game/Free_Magic/VFX_Niagara/NS_Free_Magic_Attack1.NS_Free_Magic_Attack1"));
	SlamEffect = SlamEffectFinder.Object;
}

bool UPBGolemHandSlamPattern::UsesHand(EPBGolemBossHandType TargetHandType) const
{
	return HandType == TargetHandType;
}

bool UPBGolemHandSlamPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	const APBGolemBoss* GolemBoss = Cast<APBGolemBoss>(Boss);
	APBGolemBossHand* GolemHand = GolemBoss ? GolemBoss->GetGolemHand(HandType) : nullptr;
	AActor* PinballActor = FindPinballActor(Boss);
	const bool IsCanExecute = Super::CanExecute_Implementation(Boss)
		&& GolemBoss
		&& GolemHand
		&& GolemHand->IsHandAvailable()
		&& PinballActor;

	UE_LOG(LogTemp, Log, TEXT("[GolemHandSlamPattern] CanExecute. Pattern=%s Boss=%s HandType=%d Hand=%s Pinball=%s Success=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType),
		*GetNameSafe(GolemHand),
		*GetNameSafe(PinballActor),
		IsCanExecute ? TEXT("true") : TEXT("false"));

	return IsCanExecute;
}

void UPBGolemHandSlamPattern::StartPattern_Implementation(APBBossBase* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GolemHandSlamPattern] StartPattern. Pattern=%s Boss=%s HandType=%d"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType));

	SetOwnerBoss(Boss);

	if (!TryStartSlam(Boss))
	{
		FinishHandSlamPattern();
	}
}

void UPBGolemHandSlamPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GolemHandSlamPattern] CancelPatternInternal. Pattern=%s Boss=%s HandType=%d"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		static_cast<int32>(HandType));

	ClearSlamHoldTimer();
	UnbindHandMoveFinished();
	SlamPhase = EPBGolemHandSlamPhase::None;
	SlamTargetLocation = FVector::ZeroVector;

	if (APBGolemBoss* GolemBoss = GetGolemBoss(Boss))
	{
		ReturnHandToStartTransform(GolemBoss);
	}

	if (APBGolemBoss* GolemBoss = GetGolemBoss(Boss))
	{
		if (APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType))
		{
			GolemHand->EndPatternMovementLock();
		}
	}
}

bool UPBGolemHandSlamPattern::TryStartSlam(APBBossBase* Boss)
{
	APBGolemBoss* GolemBoss = GetGolemBoss(Boss);
	AActor* PinballActor = FindPinballActor();
	APBGolemBossHand* GolemHand = GolemBoss ? GolemBoss->GetGolemHand(HandType) : nullptr;

	if (!GolemBoss || !PinballActor || !GolemHand || !TryBindHandMoveFinished())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemHandSlamPattern] TryStartSlam failed. GolemBoss=%s Pinball=%s Hand=%s BoundMovement=%s"),
			*GetNameSafe(GolemBoss),
			*GetNameSafe(PinballActor),
			*GetNameSafe(GolemHand),
			BoundMovementComponent ? TEXT("true") : TEXT("false"));
		return false;
	}

	GolemHand->BeginPatternMovementLock();
	StartHandTransform = GolemHand->GetActorTransform();
	SlamTargetLocation = PinballActor->GetActorLocation();
	SlamPhase = EPBGolemHandSlamPhase::Preparing;
	GolemHand->MoveTelegraphStartToWorldLocationForPattern(SlamTargetLocation + ReadyOffset, ReadyDuration);
	DrawDebugSlamRange(ReadyDuration + SlamDuration);

	UE_LOG(LogTemp, Log, TEXT("[GolemHandSlamPattern] Ready move requested. HandType=%d TargetLocation=%s ReadyLocation=%s ReadyDuration=%.2f"),
		static_cast<int32>(HandType),
		*SlamTargetLocation.ToString(),
		*(SlamTargetLocation + ReadyOffset).ToString(),
		ReadyDuration);

	return true;
}

void UPBGolemHandSlamPattern::HandleHandMoveFinished()
{
	UE_LOG(LogTemp, Log, TEXT("[GolemHandSlamPattern] HandleHandMoveFinished. Pattern=%s Phase=%d"),
		*GetNameSafe(this),
		static_cast<int32>(SlamPhase));

	APBGolemBoss* GolemBoss = GetGolemBoss();
	APBGolemBossHand* GolemHand = GolemBoss ? GolemBoss->GetGolemHand(HandType) : nullptr;
	if (!GolemBoss || !GolemHand)
	{
		FinishHandSlamPattern();
		return;
	}

	switch (SlamPhase)
	{
	case EPBGolemHandSlamPhase::Preparing:
		SlamPhase = EPBGolemHandSlamPhase::Slamming;
		GolemHand->MoveTelegraphStartToWorldLocationForPattern(SlamTargetLocation, SlamDuration);
		DrawDebugSlamRange(SlamDuration);
		break;
	case EPBGolemHandSlamPhase::Slamming:
		DrawDebugSlamRange(DebugSlamRangeDuration);
		ApplySlamDamage();
		SpawnSlamEffect();
		SlamPhase = EPBGolemHandSlamPhase::Holding;
		if (SlamHoldDuration <= 0.0f)
		{
			HandleSlamHoldFinished();
			break;
		}

		if (UWorld* World = GolemBoss->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				SlamHoldTimerHandle,
				this,
				&UPBGolemHandSlamPattern::HandleSlamHoldFinished,
				SlamHoldDuration,
				false);
			break;
		}

		HandleSlamHoldFinished();
		break;
	case EPBGolemHandSlamPhase::Returning:
	default:
		FinishHandSlamPattern();
		break;
	}
}

void UPBGolemHandSlamPattern::HandleSlamHoldFinished()
{
	ClearSlamHoldTimer();

	APBGolemBoss* GolemBoss = GetGolemBoss();
	if (!GolemBoss)
	{
		FinishHandSlamPattern();
		return;
	}

	SlamPhase = EPBGolemHandSlamPhase::Returning;
	ReturnHandToStartTransform(GolemBoss);
}

bool UPBGolemHandSlamPattern::TryBindHandMoveFinished()
{
	APBGolemBossHand* GolemHand = GetGolemHand(HandType);
	if (!GolemHand)
	{
		return false;
	}

	BoundMovementComponent = GolemHand->GetHandMovementComponent();
	if (!BoundMovementComponent)
	{
		return false;
	}

	BoundMovementComponent->OnMoveFinished.RemoveDynamic(this, &UPBGolemHandSlamPattern::HandleHandMoveFinished);
	BoundMovementComponent->OnMoveFinished.AddUniqueDynamic(this, &UPBGolemHandSlamPattern::HandleHandMoveFinished);
	return true;
}

void UPBGolemHandSlamPattern::UnbindHandMoveFinished()
{
	if (BoundMovementComponent)
	{
		BoundMovementComponent->OnMoveFinished.RemoveDynamic(this, &UPBGolemHandSlamPattern::HandleHandMoveFinished);
	}

	BoundMovementComponent = nullptr;
}

void UPBGolemHandSlamPattern::ClearSlamHoldTimer()
{
	const APBBossBase* Boss = GetOwnerBoss();
	UWorld* World = Boss ? Boss->GetWorld() : nullptr;
	if (World)
	{
		World->GetTimerManager().ClearTimer(SlamHoldTimerHandle);
	}
}

void UPBGolemHandSlamPattern::DrawDebugSlamRange(float Duration) const
{
	if (!IsDrawDebugSlamRange || SlamRadius <= 0.0f)
	{
		return;
	}

	const APBBossBase* Boss = GetOwnerBoss();
	UWorld* World = Boss ? Boss->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	DrawDebugSphere(
		World,
		SlamTargetLocation,
		SlamRadius,
		24,
		FColor::Red,
		false,
		FMath::Max(0.0f, Duration),
		0,
		2.0f);
}

void UPBGolemHandSlamPattern::SpawnSlamEffect() const
{
	const APBBossBase* Boss = GetOwnerBoss();
	UWorld* World = Boss ? Boss->GetWorld() : nullptr;
	if (!World || !SlamEffect || SlamRadius <= 0.0f)
	{
		return;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		SlamEffect,
		SlamTargetLocation);
	if (!NiagaraComponent)
	{
		return;
	}

	const float Scale = SlamRadius / FMath::Max(0.01f, SlamEffectRadiusAtScaleOne);
	NiagaraComponent->SetVariableFloat(TEXT("User.Scale_All"), Scale);
}

void UPBGolemHandSlamPattern::ApplySlamDamage() const
{
	AActor* PinballActor = FindPinballActor();
	if (!PinballActor || SlamRadius <= 0.0f || SlamDamage <= 0)
	{
		return;
	}

	FVector SlamToPinball = PinballActor->GetActorLocation() - SlamTargetLocation;
	SlamToPinball.Z = 0.0f;
	if (SlamToPinball.SizeSquared() > FMath::Square(SlamRadius))
	{
		return;
	}

	IDamageable* Damageable = PBInterfaceUtils::FindInterface<IDamageable>(PinballActor);
	if (!Damageable || Damageable->IsDead())
	{
		return;
	}

	Damageable->TakeDamage(SlamDamage);
	const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
	UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
		*SourcePatternName.ToString(),
		SlamDamage,
		*GetNameSafe(PinballActor));
}

void UPBGolemHandSlamPattern::ReturnHandToStartTransform(APBGolemBoss* GolemBoss)
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

void UPBGolemHandSlamPattern::FinishHandSlamPattern()
{
	UE_LOG(LogTemp, Log, TEXT("[GolemHandSlamPattern] FinishHandSlamPattern. Pattern=%s"),
		*GetNameSafe(this));

	ClearSlamHoldTimer();
	UnbindHandMoveFinished();
	if (APBGolemBoss* GolemBoss = GetGolemBoss())
	{
		if (APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType))
		{
			GolemHand->EndPatternMovementLock();
		}
	}

	SlamTargetLocation = FVector::ZeroVector;
	StartHandTransform = FTransform::Identity;
	SlamPhase = EPBGolemHandSlamPhase::None;
	FinishPattern();
}
