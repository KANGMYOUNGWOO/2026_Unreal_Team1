#include "PBBossSnakeChargePattern.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossChargeTelegraph.h"
#include "PinBallLike/Actor/Boss/SnakeBoss.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

namespace
{
	FVector CalculateSnakeChargeBezierLocation(
		const FVector& StartLocation,
		const FVector& FirstControlLocation,
		const FVector& SecondControlLocation,
		const FVector& EndLocation,
		float Alpha)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		const float InverseAlpha = 1.0f - ClampedAlpha;

		return StartLocation * InverseAlpha * InverseAlpha * InverseAlpha
			+ FirstControlLocation * 3.0f * InverseAlpha * InverseAlpha * ClampedAlpha
			+ SecondControlLocation * 3.0f * InverseAlpha * ClampedAlpha * ClampedAlpha
			+ EndLocation * ClampedAlpha * ClampedAlpha * ClampedAlpha;
	}
}

bool UPBBossSnakeChargePattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss) && ChargeSpeed > 0.0f && ChargeMaxDistance > 0.0f;
}

void UPBBossSnakeChargePattern::StartPattern_Implementation(APBBossBase* Boss)
{
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetOwnerBoss(Boss);
	StartAiming(Boss);
}

void UPBBossSnakeChargePattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	StartCharge();
}

void UPBBossSnakeChargePattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	ClearPatternTimers();
	DestroyChargeHitCollision();
	DestroyChargeTelegraph();
	SetPinballCollisionDamageBlocked(false);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakeChargeMovementCollisionEnabled(true);
	}

	if (IsChargeMovementStarted && Boss)
	{
		Boss->SetActorLocationAndRotation(
			ChargeStartLocation,
			ChargeStartRotation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);

		if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
		{
			SnakeBoss->ResetSnakeMovementPath();
		}
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::None);

	ChargeProgressDistance = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;
	ReturnElapsedSeconds = 0.0f;
	ChargeEndLocation = FVector::ZeroVector;
	ChargePathLength = 0.0f;
	IsChargeMovementStarted = false;
}

void UPBBossSnakeChargePattern::ExecuteNativePattern(APBBossBase* Boss)
{
	ExecutePattern_Implementation(Boss);
}

void UPBBossSnakeChargePattern::SetChargePatternState(EPBBossSnakeChargePatternState NewState)
{
	ChargePatternState = NewState;
}

void UPBBossSnakeChargePattern::StartAiming(APBBossBase* Boss)
{
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::Aiming);
	PrepareCharge(Boss);
	SpawnChargeTelegraph(Boss);

	const float TelegraphDurationSeconds = GetChargeTelegraphDurationSeconds();
	if (TelegraphDurationSeconds <= 0.0f || !IsValid(SpawnedChargeTelegraph))
	{
		HandleChargeTelegraphFinished(ChargeStartLocation + ChargeDirection * ChargeMaxDistance, ChargeDirection);
		return;
	}
}

void UPBBossSnakeChargePattern::HandleChargeTelegraphFinished(FVector TargetLocation, FVector Direction)
{
	DestroyChargeTelegraph();

	ChargeDirection = Direction;
	ChargeDirection.Z = 0.0f;
	ChargeDirection = ChargeDirection.GetSafeNormal();
	if (ChargeDirection.IsNearlyZero())
	{
		ChargeDirection = FVector::ForwardVector;
	}

	BuildChargePath(TargetLocation);

	if (APBBossBase* Boss = GetOwnerBoss())
	{
		ExecutePattern_Implementation(Boss);
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::None);
	FinishPattern();
}

float UPBBossSnakeChargePattern::GetChargeTelegraphDurationSeconds() const
{
	return TelegraphDataList.IsEmpty()
		? 0.0f
		: TelegraphDataList[0].DurationSeconds;
}

void UPBBossSnakeChargePattern::PrepareCharge(APBBossBase* Boss)
{
	if (!Boss)
	{
		return;
	}

	ChargeStartLocation = Boss->GetActorLocation();
	ChargeStartRotation = Boss->GetActorRotation();
	ChargeProgressDistance = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;
	ReturnElapsedSeconds = 0.0f;
	ChargeEndLocation = ChargeStartLocation;
	ReturnEndLocation = ChargeStartLocation;
	ChargePathLength = 0.0f;
	IsChargeMovementStarted = false;

	RefreshChargeDirection(Boss);
}

void UPBBossSnakeChargePattern::RefreshChargeDirection(APBBossBase* Boss)
{
	if (!Boss)
	{
		return;
	}

	if (AActor* TargetPinballActor = FindPinballActor())
	{
		ChargeDirection = TargetPinballActor->GetActorLocation() - ChargeStartLocation;
		ChargeDirection.Z = 0.0f;
		ChargeDirection = ChargeDirection.GetSafeNormal();
	}
	else
	{
		ChargeDirection = Boss->GetActorForwardVector();
		ChargeDirection.Z = 0.0f;
		ChargeDirection = ChargeDirection.GetSafeNormal();
	}

	if (ChargeDirection.IsNearlyZero())
	{
		ChargeDirection = FVector::ForwardVector;
	}
}

void UPBBossSnakeChargePattern::BuildChargePath(const FVector& TargetLocation)
{
	ChargeEndLocation = TargetLocation;
	ChargeEndLocation.Z = ChargeStartLocation.Z;

	ChargeDirection = ChargeEndLocation - ChargeStartLocation;
	ChargeDirection.Z = 0.0f;
	ChargePathLength = ChargeDirection.Size();
	ChargeDirection = ChargeDirection.GetSafeNormal();
	if (ChargeDirection.IsNearlyZero())
	{
		ChargeDirection = ChargeStartRotation.Vector();
		ChargeDirection.Z = 0.0f;
		ChargeDirection = ChargeDirection.GetSafeNormal();
	}
}

FVector UPBBossSnakeChargePattern::GetChargePathLocation(float Distance) const
{
	if (ChargePathLength <= KINDA_SMALL_NUMBER)
	{
		return ChargeStartLocation;
	}

	const float Alpha = FMath::Clamp(Distance / ChargePathLength, 0.0f, 1.0f);
	return FMath::Lerp(ChargeStartLocation, ChargeEndLocation, Alpha);
}

void UPBBossSnakeChargePattern::SpawnChargeTelegraph(APBBossBase* Boss)
{
	DestroyChargeTelegraph();

	if (!Boss || TelegraphDataList.IsEmpty())
	{
		return;
	}

	const TArray<APBBossPatternTelegraph*> SpawnedTelegraphList = SpawnTelegraph(Boss);
	for (APBBossPatternTelegraph* SpawnedTelegraph : SpawnedTelegraphList)
	{
		SpawnedChargeTelegraph = Cast<APBBossChargeTelegraph>(SpawnedTelegraph);
		if (!SpawnedChargeTelegraph)
		{
			continue;
		}

		SpawnedChargeTelegraph->OnChargeTelegraphFinished.AddUniqueDynamic(
			this,
			&UPBBossSnakeChargePattern::HandleChargeTelegraphFinished);
		break;
	}
}

void UPBBossSnakeChargePattern::StartCharge()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!IsValid(Boss))
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::Charging);
	SetPinballCollisionDamageBlocked(true);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakeChargeMovementCollisionEnabled(false);
	}
	ChargeProgressDistance = 0.0f;

	if (ChargePathLength <= KINDA_SMALL_NUMBER)
	{
		FinishReturn();
		return;
	}

	IsChargeMovementStarted = true;
	CreateChargeHitCollision(Boss);

	Boss->GetWorldTimerManager().SetTimer(
		ChargeTimerHandle,
		this,
		&UPBBossSnakeChargePattern::UpdateCharge,
		UpdateIntervalSeconds,
		true);
}

void UPBBossSnakeChargePattern::UpdateCharge()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!IsValid(Boss))
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	if (IsDrawChargeHitRange && IsValid(Boss->GetWorld()))
	{
		DrawDebugSphere(
			Boss->GetWorld(),
			Boss->GetActorLocation(),
			ChargeHitRadius,
			64,
			FColor::Blue,
			false,
			UpdateIntervalSeconds * 2.0f,
			0,
			5.0f);
	}

	const float ChargeDistance = FMath::Min(ChargeSpeed * UpdateIntervalSeconds, ChargePathLength - ChargeProgressDistance);
	if (ChargeDistance <= 0.0f)
	{
		FinishCharge();
		return;
	}

	MoveBossByChargeDistance(Boss, ChargeDistance);
	ChargeProgressDistance += ChargeDistance;

	if (ChargeProgressDistance >= ChargePathLength)
	{
		FinishCharge();
	}
}

void UPBBossSnakeChargePattern::MoveBossByChargeDistance(APBBossBase* Boss, float ChargeDistance)
{
	if (!Boss || ChargeDistance <= 0.0f)
	{
		return;
	}

	const float TargetDistance = FMath::Min(ChargeProgressDistance + ChargeDistance, ChargePathLength);
	const FVector PreviousLocation = Boss->GetActorLocation();
	const FVector TargetLocation = GetChargePathLocation(TargetDistance);

	Boss->SetActorRotation(ChargeDirection.Rotation());
	Boss->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::None);

	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->UpdateSnakeChargeMovement(
			UpdateIntervalSeconds,
			PreviousLocation,
			TargetLocation);
	}
}

void UPBBossSnakeChargePattern::FinishCharge()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ChargeTimerHandle);
	DestroyChargeHitCollision();
	PlayPatternSFX(Boss);
	StartReturn();
}

void UPBBossSnakeChargePattern::StartReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::Returning);
	ReturnEndLocation = FindEmergenceLocation(Boss);
	FVector ReturnDirection = ChargeStartRotation.Vector().GetSafeNormal2D();
	if (ReturnDirection.IsNearlyZero())
	{
		ReturnDirection = FVector::ForwardVector;
	}

	ReturnFirstControlLocation = ChargeEndLocation + ChargeDirection * ReturnCurveControlDistance;
	ReturnFirstControlLocation.Z = ChargeStartLocation.Z - UndergroundDepth;
	ReturnSecondControlLocation = ReturnEndLocation - ReturnDirection * ReturnCurveControlDistance;
	ReturnSecondControlLocation.Z = ChargeStartLocation.Z - UndergroundDepth;
	ReturnElapsedSeconds = 0.0f;
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakeChargeMovementCollisionEnabled(false);
	}
	if (ReboundSeconds <= 0.0f)
	{
		FinishReturn();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		ReturnTimerHandle,
		this,
		&UPBBossSnakeChargePattern::UpdateReturn,
		UpdateIntervalSeconds,
		true);
}

FVector UPBBossSnakeChargePattern::FindEmergenceLocation(APBBossBase* Boss) const
{
	const FVector DefaultLocation = ChargeStartLocation + FVector(ReturnUpwardOffset, 0.0f, 0.0f);
	const ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss);
	if (!SnakeBoss || EmergenceSearchRadius <= 0.0f)
	{
		return DefaultLocation;
	}

	FVector BallLocation = ChargeEndLocation;
	if (const AActor* PinballActor = FindPinballActor())
	{
		BallLocation = PinballActor->GetActorLocation();
	}

	constexpr int32 CandidateCount = 8;
	FVector BestLocation = SnakeBoss->GetClampedSnakePatrolLocation(DefaultLocation);
	float BestClearanceSquared = FMath::Min(
		FVector::DistSquared2D(BestLocation, ChargeEndLocation),
		FVector::DistSquared2D(BestLocation, BallLocation));
	const float MinimumDistanceSquared = FMath::Square(MinimumEmergenceDistance);

	for (int32 CandidateIndex = 0; CandidateIndex < CandidateCount; ++CandidateIndex)
	{
		const float AngleRadians = UE_TWO_PI * static_cast<float>(CandidateIndex) / static_cast<float>(CandidateCount);
		const FVector CandidateDirection(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f);
		const FVector CandidateLocation = SnakeBoss->GetClampedSnakePatrolLocation(
			ChargeStartLocation + CandidateDirection * EmergenceSearchRadius);
		const float CandidateClearanceSquared = FMath::Min(
			FVector::DistSquared2D(CandidateLocation, ChargeEndLocation),
			FVector::DistSquared2D(CandidateLocation, BallLocation));

		if (CandidateClearanceSquared > BestClearanceSquared)
		{
			BestLocation = CandidateLocation;
			BestClearanceSquared = CandidateClearanceSquared;
		}

		if (BestClearanceSquared >= MinimumDistanceSquared)
		{
			break;
		}
	}

	BestLocation.Z = ChargeStartLocation.Z;
	return BestLocation;
}

void UPBBossSnakeChargePattern::UpdateReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	const FVector PreviousLocation = Boss->GetActorLocation();
	ReturnElapsedSeconds = FMath::Min(ReturnElapsedSeconds + UpdateIntervalSeconds, ReboundSeconds);
	const float ReturnAlpha = FMath::Clamp(ReturnElapsedSeconds / ReboundSeconds, 0.0f, 1.0f);
	const FVector NextLocation = CalculateSnakeChargeBezierLocation(
		ChargeEndLocation,
		ReturnFirstControlLocation,
		ReturnSecondControlLocation,
		ReturnEndLocation,
		ReturnAlpha);
	Boss->SetActorLocation(NextLocation, false, nullptr, ETeleportType::None);

	const FVector ReturnDirection = (NextLocation - PreviousLocation).GetSafeNormal2D();
	if (!ReturnDirection.IsNearlyZero())
	{
		Boss->SetActorRotation(ReturnDirection.Rotation());
	}

	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->UpdateSnakeChargeMovement(
			UpdateIntervalSeconds,
			PreviousLocation,
			NextLocation);
	}

	if (ReturnAlpha >= 1.0f)
	{
		FinishReturn();
	}
}

void UPBBossSnakeChargePattern::FinishReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakeChargeMovementCollisionEnabled(true);
	}
	IsChargeMovementStarted = false;
	SetPinballCollisionDamageBlocked(false);
	SetChargePatternState(EPBBossSnakeChargePatternState::None);
	FinishPattern();
}

void UPBBossSnakeChargePattern::ClearPatternTimers()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ChargeTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
}

void UPBBossSnakeChargePattern::DestroyChargeTelegraph()
{
	if (!IsValid(SpawnedChargeTelegraph))
	{
		SpawnedChargeTelegraph = nullptr;
		DestroySpawnedTelegraphs();
		return;
	}

	SpawnedChargeTelegraph->OnChargeTelegraphFinished.RemoveDynamic(
		this,
		&UPBBossSnakeChargePattern::HandleChargeTelegraphFinished);
	SpawnedChargeTelegraph->DestroyTelegraph();
	SpawnedChargeTelegraph = nullptr;
	DestroySpawnedTelegraphs();
}

void UPBBossSnakeChargePattern::SetPinballCollisionDamageBlocked(bool IsBlocked) const
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->SetPinballCollisionDamageBlocked(IsBlocked);
	}
}

void UPBBossSnakeChargePattern::CreateChargeHitCollision(APBBossBase* Boss)
{
	DestroyChargeHitCollision();
	if (!Boss || ChargeHitRadius <= 0.0f)
	{
		return;
	}

	DamagedBalls.Reset();
	ChargeHitCollision = NewObject<USphereComponent>(Boss, TEXT("SnakeChargeHitCollision"));
	ChargeHitCollision->InitSphereRadius(ChargeHitRadius);
	ChargeHitCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChargeHitCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	ChargeHitCollision->SetGenerateOverlapEvents(true);
	ChargeHitCollision->RegisterComponent();
	ChargeHitCollision->AttachToComponent(Boss->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	ChargeHitCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &UPBBossSnakeChargePattern::HandleChargeBeginOverlap);
}

void UPBBossSnakeChargePattern::DestroyChargeHitCollision()
{
	if (ChargeHitCollision)
	{
		ChargeHitCollision->DestroyComponent();
		ChargeHitCollision = nullptr;
	}
	DamagedBalls.Reset();
}

void UPBBossSnakeChargePattern::HandleChargeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	ApplyChargeHit(Cast<APBBallBase>(OtherActor));
}

void UPBBossSnakeChargePattern::ApplyChargeHit(APBBallBase* Ball)
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss || !Ball)
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
		if (!Damageable->IsDead() && DamageAmount > 0)
		{
			Damageable->TakeDamage(DamageAmount);
			const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
			UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
				*SourcePatternName.ToString(), DamageAmount, *GetNameSafe(Ball));
		}
	}

	if (IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball))
	{
		FVector BounceDirection = Ball->GetActorLocation() - Boss->GetActorLocation();
		BounceDirection.Z = 0.0f;
		if (ChargeBounceVelocity > 0.0f && BounceDirection.Normalize())
		{
			Movable->AddVelocity(BounceDirection * ChargeBounceVelocity);
		}
	}

	if (ChargePatternState == EPBBossSnakeChargePatternState::Charging)
	{
		FinishCharge();
	}
}
