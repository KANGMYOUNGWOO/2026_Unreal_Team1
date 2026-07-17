// Fill out your copyright notice in the Description page of Project Settings.

#include "PBVelocityBoostBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"

void UPBVelocityBoostBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (!IsValid(Bumper) || !IsValid(InteractionActor))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because activation context is invalid. Bumper=%s Target=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because Power is invalid. Power=%.2f"),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const float TargetSpeed = FMath::Min(EffectData.Power, FMath::Max(MaxBoostPower, 0.0f));
	if (TargetSpeed <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because MaxBoostPower is zero. Effect=%s"),
			*GetNameSafe(this));
		FinishEffect();
		return;
	}

	if (TargetSpeed < EffectData.Power)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity target was limited. Requested=%.2f Target=%.2f"),
			EffectData.Power,
			TargetSpeed);
	}

	if (!PBInterfaceUtils::FindInterface<IMovable>(InteractionActor))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because the target has no movable interface. Target=%s"),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	const APBBumperTriggerActorBase* ActiveTrigger = Bumper->GetActiveTriggerActor();
	if (!IsValid(ActiveTrigger))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because the active Trigger is missing. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	FVector FallbackDirection = InteractionActor->GetActorLocation() - ActiveTrigger->GetActorLocation();
	FallbackDirection.Z = 0.0f;
	if (!FallbackDirection.Normalize())
	{
		FallbackDirection = ActiveTrigger->GetActorForwardVector();
		FallbackDirection.Z = 0.0f;
		FallbackDirection.Normalize();
	}

	if (FallbackDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because no valid boost direction could be resolved. Trigger=%s Target=%s"),
			*GetNameSafe(ActiveTrigger),
			*GetNameSafe(InteractionActor));
		FinishEffect();
		return;
	}

	UWorld* World = Bumper->GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Velocity boost skipped because World is invalid. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	const TWeakObjectPtr<AActor> WeakInteractionActor = InteractionActor;
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(
		[WeakInteractionActor, TargetSpeed, FallbackDirection]()
		{
			AActor* ResolvedActor = WeakInteractionActor.Get();
			if (!IsValid(ResolvedActor))
			{
				return;
			}

			IMovable* ResolvedMovable = PBInterfaceUtils::FindInterface<IMovable>(ResolvedActor);
			if (!ResolvedMovable)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[Bumper] Deferred velocity boost skipped because the target has no movable interface. Target=%s"),
					*GetNameSafe(ResolvedActor));
				return;
			}

			FVector BoostDirection = ResolvedMovable->GetVelocity().GetSafeNormal2D();
			if (BoostDirection.IsNearlyZero())
			{
				BoostDirection = FallbackDirection;
			}

			const float SpeedBefore = ResolvedMovable->GetVelocity().Size2D();
			const float AddedSpeed = FMath::Max(TargetSpeed - SpeedBefore, 0.0f);
			if (AddedSpeed > KINDA_SMALL_NUMBER)
			{
				ResolvedMovable->AddVelocity(BoostDirection * AddedSpeed);
			}
			const float SpeedAfter = ResolvedMovable->GetVelocity().Size2D();

			UE_LOG(LogTemp, Log,
				TEXT("[Bumper] Deferred velocity floor resolved. Target=%s TargetSpeed=%.2f AddedSpeed=%.2f SpeedBefore=%.2f SpeedAfter=%.2f"),
				*GetNameSafe(ResolvedActor),
				TargetSpeed,
				AddedSpeed,
				SpeedBefore,
				SpeedAfter);
		}));

	UE_LOG(LogTemp, Verbose,
		TEXT("[Bumper] Velocity floor scheduled after movement resolution. Bumper=%s Target=%s TargetSpeed=%.2f"),
		*GetNameSafe(Bumper),
		*GetNameSafe(InteractionActor),
		TargetSpeed);

	// 예약이 끝났으므로 범퍼 실행 상태는 즉시 Idle로 돌리고, 가속은 Timer가 안전하게 처리한다.
	FinishEffect();
}
