#include "PBBossSnakeChargePattern.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossChargeTelegraph.h"
#include "PinBallLike/Actor/Boss/SnakeBoss.h"

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
	DestroyChargeTelegraph();
	SetPinballCollisionDamageBlocked(false);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakePinballCollisionEnabled(true);
	}

	if (IsChargeMovementStarted && Boss)
	{
		const FVector PreviousLocation = Boss->GetActorLocation();
		Boss->SetActorLocationAndRotation(
			ChargeStartLocation,
			ChargeStartRotation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);

		if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
		{
			SnakeBoss->UpdateSnakeChargeMovement(
				UpdateIntervalSeconds,
				PreviousLocation,
				ChargeStartLocation);
		}
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::None);

	ChargeProgressDistance = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;
	GroggyEndTimeSeconds = 0.0f;
	PausedGroggyRemainingSeconds = 0.0f;
	ChargeEndLocation = FVector::ZeroVector;
	ChargePathLength = 0.0f;
	IsChargeMovementStarted = false;
}

void UPBBossSnakeChargePattern::ExecuteNativePattern(APBBossBase* Boss)
{
	ExecutePattern_Implementation(Boss);
}

bool UPBBossSnakeChargePattern::PausePatternForExternalGroggy(APBBossBase* Boss)
{
	if (!Boss || ChargePatternState != EPBBossSnakeChargePatternState::Groggy)
	{
		return false;
	}

	const UWorld* World = Boss->GetWorld();
	const float CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0f;
	PausedGroggyRemainingSeconds = FMath::Max(0.0f, GroggyEndTimeSeconds - CurrentTimeSeconds);
	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	return true;
}

bool UPBBossSnakeChargePattern::ResumePatternAfterExternalGroggy(APBBossBase* Boss)
{
	if (!Boss || ChargePatternState != EPBBossSnakeChargePatternState::Groggy)
	{
		return false;
	}

	if (PausedGroggyRemainingSeconds <= 0.0f)
	{
		FinishGroggy();
		return true;
	}

	const UWorld* World = Boss->GetWorld();
	GroggyEndTimeSeconds = World ? World->GetTimeSeconds() + PausedGroggyRemainingSeconds : 0.0f;
	Boss->GetWorldTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&UPBBossSnakeChargePattern::FinishGroggy,
		PausedGroggyRemainingSeconds,
		false);
	PausedGroggyRemainingSeconds = 0.0f;
	return true;
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
	ChargeEndLocation = ChargeStartLocation;
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
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::Charging);
	SetPinballCollisionDamageBlocked(true);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakePinballCollisionEnabled(true);
	}
	ChargeProgressDistance = 0.0f;

	if (ChargePathLength <= KINDA_SMALL_NUMBER)
	{
		FinishRebound();
		return;
	}

	IsChargeMovementStarted = true;

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
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
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
	StartRebound();
}

void UPBBossSnakeChargePattern::StartRebound()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::Rebounding);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakePinballCollisionEnabled(false);
	}
	if (ReboundSeconds <= 0.0f)
	{
		FinishRebound();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		ReboundTimerHandle,
		this,
		&UPBBossSnakeChargePattern::UpdateRebound,
		UpdateIntervalSeconds,
		true);
}

void UPBBossSnakeChargePattern::UpdateRebound()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	const float ReturnDistance = ChargePathLength * UpdateIntervalSeconds / ReboundSeconds;
	if (ReturnDistance <= 0.0f)
	{
		FinishRebound();
		return;
	}

	const FVector PreviousLocation = Boss->GetActorLocation();
	ChargeProgressDistance = FMath::Max(ChargeProgressDistance - ReturnDistance, 0.0f);
	const FVector NextLocation = GetChargePathLocation(ChargeProgressDistance);
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

	if (ChargeProgressDistance <= 0.0f)
	{
		FinishRebound();
	}
}

void UPBBossSnakeChargePattern::FinishRebound()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ReboundTimerHandle);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->SetSnakePinballCollisionEnabled(true);
	}
	IsChargeMovementStarted = false;
	StartGroggy();
}

void UPBBossSnakeChargePattern::StartGroggy()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossSnakeChargePatternState::Groggy);
	SetPinballCollisionDamageBlocked(false);
	if (GroggySeconds <= 0.0f)
	{
		FinishGroggy();
		return;
	}

	const UWorld* World = Boss->GetWorld();
	GroggyEndTimeSeconds = World ? World->GetTimeSeconds() + GroggySeconds : 0.0f;
	PausedGroggyRemainingSeconds = 0.0f;
	Boss->GetWorldTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&UPBBossSnakeChargePattern::FinishGroggy,
		GroggySeconds,
		false);
}

void UPBBossSnakeChargePattern::FinishGroggy()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	GroggyEndTimeSeconds = 0.0f;
	PausedGroggyRemainingSeconds = 0.0f;
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
	Boss->GetWorldTimerManager().ClearTimer(ReboundTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
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
