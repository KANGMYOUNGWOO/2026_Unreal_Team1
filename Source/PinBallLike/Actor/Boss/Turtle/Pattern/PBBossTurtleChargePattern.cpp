#include "PBBossTurtleChargePattern.h"

#include "Animation/AnimSequence.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/TriggerBox.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/Turtle/PBTurtleBoss.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UPBBossTurtleChargePattern::UPBBossTurtleChargePattern()
{
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ChargeAnimationFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/hide-the-turtle/source/Turtle-HideAttack3.Turtle-HideAttack3"));
	ChargeAnimation = ChargeAnimationFinder.Object;
}

bool UPBBossTurtleChargePattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss)
		&& Cast<APBTurtleBoss>(Boss)
		&& ChargeAnimation
		&& ChargeSpeed > 0.0f
		&& ChargeEndTimeSeconds > ChargeStartTimeSeconds
		&& ChargeAnimation->GetPlayLength() >= ChargeEndTimeSeconds;
}

void UPBBossTurtleChargePattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	APBTurtleBoss* TurtleBoss = Cast<APBTurtleBoss>(Boss);
	AActor* Ball = FindPinballActor(Boss);
	if (!TurtleBoss || !Ball)
	{
		FinishPattern();
		return;
	}

	FVector Direction = Ball->GetActorLocation() - TurtleBoss->GetActorLocation();
	Direction.Z = 0.0f;
	if (!Direction.Normalize())
	{
		FinishPattern();
		return;
	}

	ChargeDirection = Direction;
	IsChargeEndLocationValid = FindChargeEndLocation(TurtleBoss, ChargeEndLocation);
	FRotator ChargeRotation = ChargeDirection.Rotation();
	ChargeRotation.Pitch = 0.0f;
	ChargeRotation.Yaw += ChargeFacingYawOffset;
	ChargeRotation.Roll = 0.0f;
	TurtleBoss->SetActorRotation(ChargeRotation);
	TurtleBoss->PlayTurtleAnimation(ChargeAnimation, true);

	FTimerManager& TimerManager = TurtleBoss->GetWorldTimerManager();
	TimerManager.SetTimer(
		ChargeStartTimerHandle,
		this,
		&UPBBossTurtleChargePattern::StartCharge,
		ChargeStartTimeSeconds,
		false);
	TimerManager.SetTimer(
		ChargeStopTimerHandle,
		this,
		&UPBBossTurtleChargePattern::StopCharge,
		ChargeEndTimeSeconds,
		false);
	TimerManager.SetTimer(
		ChargeFinishTimerHandle,
		this,
		&UPBBossTurtleChargePattern::FinishCharge,
		ChargeAnimation->GetPlayLength(),
		false);
}

void UPBBossTurtleChargePattern::StartCharge()
{
	APBTurtleBoss* Boss = GetTurtleBoss();
	if (!Boss)
	{
		CompleteChargePattern();
		return;
	}

	Boss->SetPinballCollisionDamageBlocked(true);
	CreateChargeHitCollision(Boss);

	Boss->GetWorldTimerManager().SetTimer(
		ChargeUpdateTimerHandle,
		this,
		&UPBBossTurtleChargePattern::UpdateCharge,
		UpdateIntervalSeconds,
		true);
}

void UPBBossTurtleChargePattern::UpdateCharge()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss(); IsValid(Boss))
	{
		if (IsDrawChargeDamageRange && IsValid(Boss->GetWorld()))
		{
			DrawDebugSphere(
				Boss->GetWorld(),
				Boss->GetActorLocation(),
				ChargeCollisionRadius,
				64,
				FColor::Blue,
				false,
				UpdateIntervalSeconds * 2.0f,
				0,
				ChargeDamageRangeLineThickness);
		}

		float MoveDistance = ChargeSpeed * UpdateIntervalSeconds;
		if (IsChargeEndLocationValid)
		{
			const float RemainingDistance = FVector::Dist2D(Boss->GetActorLocation(), ChargeEndLocation);
			MoveDistance = FMath::Min(MoveDistance, RemainingDistance);
		}

		Boss->SetActorLocation(
			Boss->GetActorLocation() + ChargeDirection * MoveDistance,
			false,
			nullptr,
			ETeleportType::None);

		if (IsChargeEndLocationValid
			&& FVector::DistSquared2D(Boss->GetActorLocation(), ChargeEndLocation) <= KINDA_SMALL_NUMBER)
		{
			Boss->SetActorLocation(ChargeEndLocation, false, nullptr, ETeleportType::None);
			PlayPatternSFX(Boss);
			StopCharge();
		}
	}
}

void UPBBossTurtleChargePattern::StopCharge()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(ChargeUpdateTimerHandle);
		Boss->SetPinballCollisionDamageBlocked(false);
	}

	DestroyChargeHitCollision();
}
void UPBBossTurtleChargePattern::FinishCharge()
{
	APBTurtleBoss* Boss = GetTurtleBoss();
	if (!Boss)
	{
		CompleteChargePattern();
		return;
	}

	StopCharge();
	Boss->RestoreTurtleAnimationMode();

	ReturnTargetLocation = Boss->GetClosestMoveAreaLocation(Boss->GetActorLocation());
	if (FVector::DistSquared2D(Boss->GetActorLocation(), ReturnTargetLocation)
		<= FMath::Square(ReturnAcceptanceRadius))
	{
		CompleteChargePattern();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		ReturnUpdateTimerHandle,
		this,
		&UPBBossTurtleChargePattern::UpdateReturnToMoveArea,
		UpdateIntervalSeconds,
		true);
}

void UPBBossTurtleChargePattern::UpdateReturnToMoveArea()
{
	APBTurtleBoss* Boss = GetTurtleBoss();
	if (!Boss)
	{
		CompleteChargePattern();
		return;
	}

	const FVector NextLocation = FMath::VInterpConstantTo(
		Boss->GetActorLocation(),
		ReturnTargetLocation,
		UpdateIntervalSeconds,
		ChargeSpeed);
	Boss->SetActorLocation(NextLocation, false, nullptr, ETeleportType::None);

	if (FVector::DistSquared2D(NextLocation, ReturnTargetLocation)
		<= FMath::Square(ReturnAcceptanceRadius))
	{
		Boss->SetActorLocation(ReturnTargetLocation, false, nullptr, ETeleportType::None);
		CompleteChargePattern();
	}
}

void UPBBossTurtleChargePattern::CompleteChargePattern()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss())
	{
		Boss->SetActorRotation(FRotator::ZeroRotator);
		Boss->SetPinballCollisionDamageBlocked(false);
	}

	CleanupCharge();
	FinishPattern();
}

void UPBBossTurtleChargePattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	if (APBTurtleBoss* Turtle = GetTurtleBoss())
	{
		Turtle->SetActorRotation(FRotator::ZeroRotator);
		Turtle->SetPinballCollisionDamageBlocked(false);
		Turtle->RestoreTurtleAnimationMode();
	}

	CleanupCharge();
}

void UPBBossTurtleChargePattern::CleanupCharge()
{
	if (APBTurtleBoss* Boss = GetTurtleBoss())
	{
		FTimerManager& TimerManager = Boss->GetWorldTimerManager();
		TimerManager.ClearTimer(ChargeStartTimerHandle);
		TimerManager.ClearTimer(ChargeUpdateTimerHandle);
		TimerManager.ClearTimer(ChargeStopTimerHandle);
		TimerManager.ClearTimer(ChargeFinishTimerHandle);
		TimerManager.ClearTimer(ReturnUpdateTimerHandle);
	}

	DestroyChargeHitCollision();
	IsChargeEndLocationValid = false;
}

APBTurtleBoss* UPBBossTurtleChargePattern::GetTurtleBoss() const
{
	return Cast<APBTurtleBoss>(GetOwnerBoss());
}

void UPBBossTurtleChargePattern::CreateChargeHitCollision(APBTurtleBoss* Boss)
{
	DestroyChargeHitCollision();
	if (!Boss || ChargeCollisionRadius <= 0.0f)
	{
		return;
	}

	ChargeHitCollision = NewObject<USphereComponent>(Boss, TEXT("TurtleChargeHitCollision"));
	ChargeHitCollision->InitSphereRadius(ChargeCollisionRadius);
	ChargeHitCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChargeHitCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	ChargeHitCollision->SetGenerateOverlapEvents(true);
	ChargeHitCollision->RegisterComponent();
	ChargeHitCollision->AttachToComponent(
		Boss->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	ChargeHitCollision->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UPBBossTurtleChargePattern::HandleChargeBeginOverlap);
}

void UPBBossTurtleChargePattern::DestroyChargeHitCollision()
{
	if (ChargeHitCollision)
	{
		ChargeHitCollision->DestroyComponent();
		ChargeHitCollision = nullptr;
	}

	DamagedBalls.Reset();
}

bool UPBBossTurtleChargePattern::FindChargeEndLocation(
	const APBTurtleBoss* Boss,
	FVector& OutChargeEndLocation) const
{
	if (!Boss)
	{
		return false;
	}

	TArray<AActor*> TriggerBoxes;
	UGameplayStatics::GetAllActorsOfClass(Boss->GetWorld(), ATriggerBox::StaticClass(), TriggerBoxes);
	float ClosestHitDistance = TNumericLimits<float>::Max();
	for (AActor* TriggerBoxActor : TriggerBoxes)
	{
		if (!TriggerBoxActor
			|| (!TriggerBoxActor->GetName().Contains(TEXT("TriggerBox"))
				&& !TriggerBoxActor->ActorHasTag(ChargeBoundaryActorName)))
		{
			continue;
		}

		const ATriggerBox* TriggerBox = Cast<ATriggerBox>(TriggerBoxActor);
		const UBoxComponent* BoxComponent = TriggerBox ? Cast<UBoxComponent>(TriggerBox->GetCollisionComponent()) : nullptr;
		if (!BoxComponent)
		{
			continue;
		}

		const FTransform BoxTransform = BoxComponent->GetComponentTransform();
		const FVector LocalStart = BoxTransform.InverseTransformPosition(Boss->GetActorLocation());
		const FVector BoxExtent = BoxComponent->GetUnscaledBoxExtent();
		const FVector LocalDirection = BoxTransform.InverseTransformVector(ChargeDirection);
		float NearDistance = 0.0f;
		float FarDistance = TNumericLimits<float>::Max();
		bool IsIntersecting = true;

		for (int32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
		{
			const float StartValue = LocalStart[AxisIndex];
			const float DirectionValue = LocalDirection[AxisIndex];
			const float ExtentValue = BoxExtent[AxisIndex];
			if (FMath::IsNearlyZero(DirectionValue))
			{
				if (StartValue < -ExtentValue || StartValue > ExtentValue)
				{
					IsIntersecting = false;
					break;
				}
				continue;
			}

			float AxisNearDistance = (-ExtentValue - StartValue) / DirectionValue;
			float AxisFarDistance = (ExtentValue - StartValue) / DirectionValue;
			if (AxisNearDistance > AxisFarDistance)
			{
				Swap(AxisNearDistance, AxisFarDistance);
			}

			NearDistance = FMath::Max(NearDistance, AxisNearDistance);
			FarDistance = FMath::Min(FarDistance, AxisFarDistance);
			if (NearDistance > FarDistance)
			{
				IsIntersecting = false;
				break;
			}
		}

		if (IsIntersecting && FarDistance >= 0.0f)
		{
			ClosestHitDistance = FMath::Min(ClosestHitDistance, NearDistance);
		}
	}

	if (ClosestHitDistance == TNumericLimits<float>::Max())
	{
		return false;
	}

	const float ChargeDistance = FMath::Max(0.0f, ClosestHitDistance - ChargeBoundaryPadding);
	OutChargeEndLocation = Boss->GetActorLocation() + ChargeDirection * ChargeDistance;
	return true;
}

void UPBBossTurtleChargePattern::HandleChargeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	ApplyChargeHit(Cast<APBBallBase>(OtherActor));
}

void UPBBossTurtleChargePattern::ApplyChargeHit(APBBallBase* Ball)
{
	if (!Ball || DamageAmount <= 0)
	{
		return;
	}

	const TObjectKey<APBBallBase> BallKey(Ball);
	if (DamagedBalls.Contains(BallKey))
	{
		return;
	}

	DamagedBalls.Add(BallKey);
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
}
