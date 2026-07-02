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
	SetPinballMoveIgnored(false);
	SetPinballCollisionDamageBlocked(false);
	SetChargePatternState(EPBBossSnakeChargePatternState::None);
	TargetPinballActor = nullptr;
	ReboundedDistance = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;
	GroggyEndTimeSeconds = 0.0f;
	PausedGroggyRemainingSeconds = 0.0f;
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
	if (TelegraphDurationSeconds <= 0.0f)
	{
		FinishAiming();
		return;
	}

	StartChargeAim(TelegraphDurationSeconds * 0.5f);

	Boss->GetWorldTimerManager().SetTimer(
		ChargeTelegraphTimerHandle,
		this,
		&UPBBossSnakeChargePattern::FinishAiming,
		TelegraphDurationSeconds,
		false);
}

void UPBBossSnakeChargePattern::FinishAiming()
{
	ClearChargeAimTimers();
	ClearChargeTelegraphTimer();
	DestroyChargeTelegraph();
	SetChargePatternState(EPBBossSnakeChargePatternState::Charging);

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
	ChargedDistance = 0.0f;
	ReboundedDistance = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;

	TargetPinballActor = FindPinballActor();
	RefreshChargeDirection(Boss);
}

void UPBBossSnakeChargePattern::RefreshChargeDirection(APBBossBase* Boss)
{
	if (!Boss)
	{
		return;
	}

	if (IsValid(TargetPinballActor))
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
		ChargeDirection = Boss->GetActorForwardVector();
		ChargeDirection.Z = 0.0f;
		ChargeDirection = ChargeDirection.GetSafeNormal();
	}

	if (ChargeDirection.IsNearlyZero())
	{
		ChargeDirection = FVector::ForwardVector;
	}
}

void UPBBossSnakeChargePattern::SpawnChargeTelegraph(APBBossBase* Boss)
{
	DestroyChargeTelegraph();

	if (!Boss || TelegraphDataList.IsEmpty())
	{
		return;
	}

	UClass* TelegraphClass = TelegraphDataList[0].TelegraphClass.Get();
	if (!TelegraphClass || !TelegraphClass->IsChildOf(APBBossChargeTelegraph::StaticClass()))
	{
		return;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Boss;
	SpawnParameters.Instigator = Boss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedChargeTelegraph = World->SpawnActor<APBBossChargeTelegraph>(
		TelegraphClass,
		ChargeStartLocation,
		ChargeDirection.Rotation(),
		SpawnParameters);

	if (SpawnedChargeTelegraph)
	{
		SpawnedChargeTelegraph->InitChargeTelegraph(
			TelegraphDataList[0].DurationSeconds,
			ChargeStartLocation,
			ChargeDirection,
			ChargeMaxDistance,
			TelegraphDataList[0].Scale);
	}
}

void UPBBossSnakeChargePattern::StartChargeAim(float AimDurationSeconds)
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss || AimDurationSeconds <= 0.0f)
	{
		return;
	}

	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = AimDurationSeconds;
	UpdateChargeAim();

	Boss->GetWorldTimerManager().SetTimer(
		ChargeAimTimerHandle,
		this,
		&UPBBossSnakeChargePattern::UpdateChargeAim,
		UpdateIntervalSeconds,
		true);

	Boss->GetWorldTimerManager().SetTimer(
		ChargeAimFinishTimerHandle,
		this,
		&UPBBossSnakeChargePattern::FinishChargeAim,
		AimDurationSeconds,
		false);
}

void UPBBossSnakeChargePattern::UpdateChargeAim()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return;
	}

	RefreshChargeDirection(Boss);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->FaceHeadDirection(ChargeDirection);
		const float PullAlpha = ChargeAimDurationSeconds > 0.0f
			? ChargeAimElapsedSeconds / ChargeAimDurationSeconds
			: 1.0f;
		SnakeBoss->PullBodyToHead(PullAlpha);
	}

	ChargeAimElapsedSeconds = FMath::Min(
		ChargeAimElapsedSeconds + UpdateIntervalSeconds,
		ChargeAimDurationSeconds);

	UpdateChargeTelegraph();
}

void UPBBossSnakeChargePattern::FinishChargeAim()
{
	ChargeAimElapsedSeconds = ChargeAimDurationSeconds;
	UpdateChargeAim();
	ClearChargeAimTimers();
}

void UPBBossSnakeChargePattern::ClearChargeAimTimers()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(ChargeAimTimerHandle);
		Boss->GetWorldTimerManager().ClearTimer(ChargeAimFinishTimerHandle);
	}
}

void UPBBossSnakeChargePattern::UpdateChargeTelegraph() const
{
	if (!IsValid(SpawnedChargeTelegraph))
	{
		return;
	}

	SpawnedChargeTelegraph->UpdateChargeTelegraphTransform(
		ChargeStartLocation,
		ChargeDirection,
		ChargeMaxDistance);
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
	SetPinballMoveIgnored(true);
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		SnakeBoss->FaceHeadDirection(ChargeDirection);
		SnakeBoss->CollapseBodyToHead();
	}

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

	const float MoveDistance = FMath::Min(ChargeSpeed * UpdateIntervalSeconds, ChargeMaxDistance - ChargedDistance);
	if (MoveDistance <= 0.0f)
	{
		HandleChargeBlocked(FHitResult());
		return;
	}

	FHitResult Hit;
	MoveBossWithFloorIgnored(Boss, ChargeDirection * MoveDistance, Hit);
	ChargedDistance += MoveDistance;

	if (Hit.bBlockingHit || ChargedDistance >= ChargeMaxDistance)
	{
		HandleChargeBlocked(Hit);
	}
}

void UPBBossSnakeChargePattern::HandleChargeBlocked(const FHitResult& Hit)
{
	static_cast<void>(Hit);

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
	ReboundedDistance = 0.0f;
	if (ReboundDistance <= 0.0f || ReboundSeconds <= 0.0f)
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

	const float ReboundSpeed = ReboundDistance / ReboundSeconds;
	const float MoveDistance = FMath::Min(ReboundSpeed * UpdateIntervalSeconds, ReboundDistance - ReboundedDistance);
	if (MoveDistance <= 0.0f)
	{
		FinishRebound();
		return;
	}

	FHitResult Hit;
	MoveBossWithFloorIgnored(Boss, -ChargeDirection * MoveDistance, Hit);
	ReboundedDistance += MoveDistance;

	if (Hit.bBlockingHit || ReboundedDistance >= ReboundDistance)
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
	SetPinballCollisionDamageBlocked(true);

	if (ReturnSpeed <= 0.0f)
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

void UPBBossSnakeChargePattern::UpdateReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossSnakeChargePatternState::None);
		FinishPattern();
		return;
	}

	const FVector CurrentLocation = Boss->GetActorLocation();
	FVector ReturnDirection = ChargeStartLocation - CurrentLocation;
	ReturnDirection.Z = 0.0f;

	const float RemainingDistance = ReturnDirection.Size();
	if (RemainingDistance <= ReturnAcceptanceRadius)
	{
		FinishReturn();
		return;
	}

	ReturnDirection = ReturnDirection.GetSafeNormal();

	const float MoveDistance = FMath::Min(ReturnSpeed * UpdateIntervalSeconds, RemainingDistance);
	FHitResult Hit;
	MoveBossWithFloorIgnored(Boss, ReturnDirection * MoveDistance, Hit);

	if (Hit.bBlockingHit)
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
	SetPinballMoveIgnored(false);
	Boss->SetActorLocation(ChargeStartLocation, false);
	Boss->SetActorRotation(ChargeStartRotation);
	SetPinballCollisionDamageBlocked(false);
	SetChargePatternState(EPBBossSnakeChargePatternState::None);
	TargetPinballActor = nullptr;
	FinishPattern();
}

bool UPBBossSnakeChargePattern::MoveBossWithFloorIgnored(APBBossBase* Boss, const FVector& MoveOffset, FHitResult& OutHit) const
{
	if (!Boss)
	{
		return false;
	}

	const FVector PreviousLocation = Boss->GetActorLocation();
	Boss->AddActorWorldOffset(MoveOffset, true, &OutHit);

	if (!IsFloorHit(OutHit))
	{
		return OutHit.bBlockingHit;
	}

	Boss->SetActorLocation(PreviousLocation + MoveOffset, false);
	OutHit = FHitResult();
	return false;
}

bool UPBBossSnakeChargePattern::IsFloorHit(const FHitResult& Hit) const
{
	if (!Hit.bBlockingHit)
	{
		return false;
	}

	const AActor* HitActor = Hit.GetActor();
	const FString HitActorName = IsValid(HitActor) ? HitActor->GetName() : FString();
	return Hit.ImpactNormal.Z > 0.5f || HitActorName.Contains(TEXT("Floor"));
}

void UPBBossSnakeChargePattern::ClearPatternTimers()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return;
	}

	ClearChargeTelegraphTimer();
	ClearChargeAimTimers();
	Boss->GetWorldTimerManager().ClearTimer(ChargeTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(ReboundTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
}

void UPBBossSnakeChargePattern::ClearChargeTelegraphTimer()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(ChargeTelegraphTimerHandle);
	}
}

void UPBBossSnakeChargePattern::DestroyChargeTelegraph()
{
	if (!IsValid(SpawnedChargeTelegraph))
	{
		SpawnedChargeTelegraph = nullptr;
		return;
	}

	SpawnedChargeTelegraph->DestroyTelegraph();
	SpawnedChargeTelegraph = nullptr;
}

void UPBBossSnakeChargePattern::SetPinballMoveIgnored(bool IsIgnored) const
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss || !TargetPinballActor)
	{
		return;
	}

	if (IsIgnored)
	{
		Boss->MoveIgnoreActorAdd(TargetPinballActor);
	}
	else
	{
		Boss->MoveIgnoreActorRemove(TargetPinballActor);
	}
}

void UPBBossSnakeChargePattern::SetPinballCollisionDamageBlocked(bool IsBlocked) const
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->SetPinballCollisionDamageBlocked(IsBlocked);
	}
}
