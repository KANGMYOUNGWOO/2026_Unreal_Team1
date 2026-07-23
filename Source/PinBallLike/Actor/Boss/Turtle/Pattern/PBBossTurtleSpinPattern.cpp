#include "PBBossTurtleSpinPattern.h"

#include "Animation/AnimSequence.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/Turtle/PBTurtleBoss.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UPBBossTurtleSpinPattern::UPBBossTurtleSpinPattern()
{
	static ConstructorHelpers::FObjectFinder<UAnimSequence> HideAnimationFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/hide-the-turtle/source/Turtle-HideHide.Turtle-HideHide"));
	HideAnimation = HideAnimationFinder.Object;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> SpinEffectFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/Niagara/Boss_Spin.Boss_Spin"));
	SpinEffect = SpinEffectFinder.Object;
}

bool UPBBossTurtleSpinPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss)
		&& Cast<APBTurtleBoss>(Boss)
		&& HideAnimation
		&& SpinDurationSeconds > 0.0f;
}

void UPBBossTurtleSpinPattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	APBTurtleBoss* TurtleBoss = Cast<APBTurtleBoss>(Boss);
	if (!TurtleBoss)
	{
		FinishPattern();
		return;
	}

	OriginalBossRotation = TurtleBoss->GetActorRotation();
	TurtleBoss->PlayTurtleAnimation(HideAnimation);
	FTimerManager& TimerManager = TurtleBoss->GetWorldTimerManager();
	TimerManager.SetTimer(SpinStartTimerHandle, this, &UPBBossTurtleSpinPattern::StartSpin, 1.12f, false);
	TimerManager.SetTimer(SpinStopTimerHandle, this, &UPBBossTurtleSpinPattern::StopSpin, 1.12f + SpinDurationSeconds, false);
	TimerManager.SetTimer(PatternCompleteTimerHandle, this, &UPBBossTurtleSpinPattern::CompletePattern, 2.12f + SpinDurationSeconds, false);
}

void UPBBossTurtleSpinPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	CleanupSpin();
}

bool UPBBossTurtleSpinPattern::PausePatternForExternalGroggy(APBBossBase* Boss)
{
	CleanupSpin();
	return true;
}

bool UPBBossTurtleSpinPattern::ResumePatternAfterExternalGroggy(APBBossBase* Boss)
{
	if (Boss)
	{
		Boss->SetActorRotation(OriginalBossRotation);
	}

	FinishPattern();
	return true;
}

void UPBBossTurtleSpinPattern::StartSpin()
{
	APBTurtleBoss* Boss = GetTurtleBoss();
	if (!Boss)
	{
		CompletePattern();
		return;
	}

	CreateSpinHitCollision(Boss);

	if (SpinEffect)
	{
		SpinEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			SpinEffect,
			Boss->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false);

		if (SpinEffectComponent)
		{
			const float EffectScale =
				(SpinCollisionRadius / FMath::Max(0.01f, SpinEffectRadiusAtScaleOne)) * SpinEffectScale;
			SpinEffectComponent->SetRelativeScale3D(FVector(EffectScale));
			SpinEffectComponent->SetVariableFloat(TEXT("User.EmitterScale"), EffectScale);
		}
	}

	Boss->GetWorldTimerManager().SetTimer(
		SpinUpdateTimerHandle,
		this,
		&UPBBossTurtleSpinPattern::UpdateSpin,
		0.016f,
		true);
}

void UPBBossTurtleSpinPattern::UpdateSpin()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss(); IsValid(Boss))
	{
		Boss->AddActorLocalRotation(FRotator(0.0f, SpinRotationSpeed * 0.016f, 0.0f));

		if (IsDrawSpinDamageRange && IsValid(Boss->GetWorld()))
		{
			DrawDebugSphere(
				Boss->GetWorld(),
				Boss->GetActorLocation(),
				SpinCollisionRadius,
				64,
				FColor::Blue,
				false,
				0.05f,
				0,
				SpinDamageRangeLineThickness);
		}
	}
}

void UPBBossTurtleSpinPattern::StopSpin()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(SpinUpdateTimerHandle);
	}

	DestroySpinHitCollision();
	if (SpinEffectComponent)
	{
		SpinEffectComponent->Deactivate();
		SpinEffectComponent = nullptr;
	}

}

void UPBBossTurtleSpinPattern::CompletePattern()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss())
	{
		Boss->SetActorRotation(OriginalBossRotation);
	}

	CleanupSpin();
	FinishPattern();
}

void UPBBossTurtleSpinPattern::CleanupSpin()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss())
	{
		FTimerManager& TimerManager = Boss->GetWorldTimerManager();
		TimerManager.ClearTimer(SpinStartTimerHandle);
		TimerManager.ClearTimer(SpinUpdateTimerHandle);
		TimerManager.ClearTimer(SpinStopTimerHandle);
		TimerManager.ClearTimer(PatternCompleteTimerHandle);
		Boss->RestoreTurtleAnimationMode();
	}

	DestroySpinHitCollision();

	if (SpinEffectComponent)
	{
		SpinEffectComponent->Deactivate();
		SpinEffectComponent = nullptr;
	}

}

APBTurtleBoss* UPBBossTurtleSpinPattern::GetTurtleBoss() const
{
	return Cast<APBTurtleBoss>(GetOwnerBoss());
}

void UPBBossTurtleSpinPattern::CreateSpinHitCollision(APBTurtleBoss* Boss)
{
	DestroySpinHitCollision();
	if (!Boss || SpinCollisionRadius <= 0.0f)
	{
		return;
	}

	SpinHitCollision = NewObject<USphereComponent>(Boss, TEXT("TurtleSpinHitCollision"));
	SpinHitCollision->InitSphereRadius(SpinCollisionRadius);
	SpinHitCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpinHitCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	SpinHitCollision->SetGenerateOverlapEvents(true);
	SpinHitCollision->RegisterComponent();
	SpinHitCollision->AttachToComponent(
		Boss->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SpinHitCollision->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UPBBossTurtleSpinPattern::HandleSpinBeginOverlap);
}

void UPBBossTurtleSpinPattern::DestroySpinHitCollision()
{
	if (SpinHitCollision)
	{
		SpinHitCollision->DestroyComponent();
		SpinHitCollision = nullptr;
	}

}

void UPBBossTurtleSpinPattern::HandleSpinBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	ApplySpinHit(Cast<APBBallBase>(OtherActor));
}

void UPBBossTurtleSpinPattern::ApplySpinHit(APBBallBase* Ball)
{
	if (!Ball || DamageAmount <= 0)
	{
		return;
	}

	if (IDamageable* Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Ball))
	{
		if (!Damageable->IsDead())
		{
			Damageable->TakeDamage(DamageAmount);
			const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
			UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
				*SourcePatternName.ToString(), DamageAmount, *GetNameSafe(Ball));
		}
	}

	APBTurtleBoss* Boss = GetTurtleBoss();
	if (Boss && SpinBounceVelocity > 0.0f)
	{
		if (IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball))
		{
			FVector BounceDirection = Ball->GetActorLocation() - Boss->GetActorLocation();
			BounceDirection.Z = 0.0f;
			if (BounceDirection.Normalize())
			{
				Movable->AddVelocity(BounceDirection * SpinBounceVelocity);
			}
		}
	}
}
