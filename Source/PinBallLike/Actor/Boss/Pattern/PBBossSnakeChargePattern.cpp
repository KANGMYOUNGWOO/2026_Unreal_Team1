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
	ApplySnakeChargePose(Boss, 0.0f);
	SetChargePatternState(EPBBossSnakeChargePatternState::None);

	ChargeProgressDistance = 0.0f;
	ReboundProgressAlpha = 0.0f;
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

	StartChargeAim(TelegraphDurationSeconds);
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
	ChargeProgressDistance = 0.0f;
	ReboundProgressAlpha = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;

	RefreshChargeDirection(Boss);
	ApplySnakeChargePose(Boss, 0.0f);
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

void UPBBossSnakeChargePattern::ApplySnakeChargePose(APBBossBase* Boss, float Alpha) const
{
	if (ASnakeBoss* SnakeBoss = Cast<ASnakeBoss>(Boss))
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		SnakeBoss->SetSnakeChargePose(ClampedAlpha > 0.0f, ChargeDirection, ClampedAlpha);
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
}

void UPBBossSnakeChargePattern::UpdateChargeAim()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return;
	}

	RefreshChargeDirection(Boss);
	ApplySnakeChargePose(Boss, 0.0f);
	ChargeAimElapsedSeconds = FMath::Min(ChargeAimElapsedSeconds + UpdateIntervalSeconds, ChargeAimDurationSeconds);
	UpdateChargeTelegraph();
}

void UPBBossSnakeChargePattern::ClearChargeAimTimers()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(ChargeAimTimerHandle);
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
	ChargeStartLocation = Boss->GetActorLocation();
	ChargeProgressDistance = 0.0f;
	ApplySnakeChargePose(Boss, 0.0f);

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

	const float ChargeDistance = FMath::Min(ChargeSpeed * UpdateIntervalSeconds, ChargeMaxDistance - ChargeProgressDistance);
	if (ChargeDistance <= 0.0f)
	{
		HandleChargeBlocked(FHitResult());
		return;
	}

	float MovedDistance = 0.0f;
	FHitResult HitResult;
	if (!MoveBossByChargeDistance(Boss, ChargeDistance, MovedDistance, HitResult))
	{
		ChargeProgressDistance += MovedDistance;
		const float ChargeAlpha = ChargeMaxDistance > 0.0f
			? ChargeProgressDistance / ChargeMaxDistance
			: 1.0f;
		ApplySnakeChargePose(Boss, ChargeAlpha);
		HandleChargeBlocked(HitResult);
		return;
	}

	ChargeProgressDistance += MovedDistance;
	const float ChargeAlpha = ChargeMaxDistance > 0.0f
		? ChargeProgressDistance / ChargeMaxDistance
		: 1.0f;
	ApplySnakeChargePose(Boss, ChargeAlpha);

	if (ChargeProgressDistance >= ChargeMaxDistance)
	{
		HandleChargeBlocked(FHitResult());
	}
}

bool UPBBossSnakeChargePattern::MoveBossByChargeDistance(
	APBBossBase* Boss,
	float ChargeDistance,
	float& OutMovedDistance,
	FHitResult& OutHitResult)
{
	OutMovedDistance = 0.0f;
	if (!Boss || ChargeDistance <= 0.0f)
	{
		return false;
	}

	const FVector PreviousLocation = Boss->GetActorLocation();
	const FVector NextLocation = Boss->GetActorLocation() + ChargeDirection * ChargeDistance;
	const bool IsMoveCompleted = Boss->SetActorLocation(NextLocation, true, &OutHitResult, ETeleportType::None);
	OutMovedDistance = FVector::Dist2D(PreviousLocation, Boss->GetActorLocation());

	if (!IsMoveCompleted || OutHitResult.IsValidBlockingHit())
	{
		return false;
	}

	Boss->SetActorRotation(ChargeDirection.Rotation());
	return true;
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
	ReboundProgressAlpha = 0.0f;
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

	const float ReboundAlpha = UpdateIntervalSeconds / ReboundSeconds;
	if (ReboundAlpha <= 0.0f)
	{
		FinishRebound();
		return;
	}

	ReboundProgressAlpha = FMath::Min(ReboundProgressAlpha + ReboundAlpha, 1.0f);
	ApplySnakeChargePose(Boss, 1.0f - ReboundProgressAlpha);

	if (ReboundProgressAlpha >= 1.0f)
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
	ApplySnakeChargePose(Boss, 0.0f);
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
	ApplySnakeChargePose(Boss, 0.0f);
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

	ClearChargeTelegraphTimer();
	ClearChargeAimTimers();
	Boss->GetWorldTimerManager().ClearTimer(ChargeTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(ReboundTimerHandle);
	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
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

void UPBBossSnakeChargePattern::SetPinballCollisionDamageBlocked(bool IsBlocked) const
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->SetPinballCollisionDamageBlocked(IsBlocked);
	}
}
