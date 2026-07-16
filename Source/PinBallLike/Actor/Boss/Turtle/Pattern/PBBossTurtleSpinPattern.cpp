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
	TurtleBoss = Cast<APBTurtleBoss>(Boss);
	if (!TurtleBoss.IsValid())
	{
		FinishPattern();
		return;
	}

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

void UPBBossTurtleSpinPattern::StartSpin()
{
	APBTurtleBoss* Boss = TurtleBoss.Get();
	if (!Boss)
	{
		CompletePattern();
		return;
	}

	DamagedBalls.Reset();
	SpinCollision = NewObject<USphereComponent>(Boss, TEXT("SpinCollision"));
	SpinCollision->InitSphereRadius(SpinCollisionRadius);
	SpinCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpinCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	SpinCollision->SetGenerateOverlapEvents(true);
	SpinCollision->RegisterComponent();
	SpinCollision->AttachToComponent(Boss->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SpinCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &UPBBossTurtleSpinPattern::HandleSpinBeginOverlap);

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
	if (APBTurtleBoss* Boss = TurtleBoss.Get())
	{
		Boss->AddActorLocalRotation(FRotator(0.0f, SpinRotationSpeed * 0.016f, 0.0f));

		if (IsDrawSpinDamageRange)
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
	if (APBTurtleBoss* Boss = TurtleBoss.Get())
	{
		Boss->GetWorldTimerManager().ClearTimer(SpinUpdateTimerHandle);
	}

	if (SpinCollision)
	{
		SpinCollision->DestroyComponent();
		SpinCollision = nullptr;
	}

	if (SpinEffectComponent)
	{
		SpinEffectComponent->Deactivate();
		SpinEffectComponent = nullptr;
	}

	DamagedBalls.Reset();
}

void UPBBossTurtleSpinPattern::CompletePattern()
{
	if (APBTurtleBoss* Boss = TurtleBoss.Get())
	{
		FRotator BossRotation = Boss->GetActorRotation();
		BossRotation.Yaw = 0.0f;
		Boss->SetActorRotation(BossRotation);
	}

	CleanupSpin();
	FinishPattern();
}

void UPBBossTurtleSpinPattern::CleanupSpin()
{
	if (APBTurtleBoss* Boss = TurtleBoss.Get())
	{
		FTimerManager& TimerManager = Boss->GetWorldTimerManager();
		TimerManager.ClearTimer(SpinStartTimerHandle);
		TimerManager.ClearTimer(SpinUpdateTimerHandle);
		TimerManager.ClearTimer(SpinStopTimerHandle);
		TimerManager.ClearTimer(PatternCompleteTimerHandle);
		Boss->RestoreTurtleAnimationMode();
	}

	if (SpinCollision)
	{
		SpinCollision->DestroyComponent();
		SpinCollision = nullptr;
	}

	if (SpinEffectComponent)
	{
		SpinEffectComponent->Deactivate();
		SpinEffectComponent = nullptr;
	}

	TurtleBoss.Reset();
	DamagedBalls.Reset();
}

void UPBBossTurtleSpinPattern::HandleSpinBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	ApplySpinDamage(Cast<APBBallBase>(OtherActor));
}

void UPBBossTurtleSpinPattern::ApplySpinDamage(APBBallBase* Ball)
{
	if (!Ball || SpinDamage <= 0)
	{
		return;
	}

	const TObjectKey<APBBallBase> BallKey(Ball);
	if (DamagedBalls.Contains(BallKey))
	{
		return;
	}

	DamagedBalls.Add(BallKey);
	IDamageable* Damageable = Cast<IDamageable>(Ball);
	if (!Damageable)
	{
		Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Ball);
	}

	if (Damageable && !Damageable->IsDead())
	{
		Damageable->TakeDamage(SpinDamage);
		const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
		UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
			*SourcePatternName.ToString(),
			SpinDamage,
			*GetNameSafe(Ball));
	}

	APBTurtleBoss* Boss = TurtleBoss.Get();
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
