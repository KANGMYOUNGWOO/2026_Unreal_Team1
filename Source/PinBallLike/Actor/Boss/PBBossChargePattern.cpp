#include "PBBossChargePattern.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossChargeTelegraph.h"
#include "PinBallLike/Actor/Boss/SnakeBoss.h"

bool UPBBossChargePattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss) && ChargeSpeed > 0.0f && ChargeMaxDistance > 0.0f;
}

void UPBBossChargePattern::StartPattern_Implementation(APBBossBase* Boss)
{
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	SetOwnerBoss(Boss);
	StartAiming(Boss);
}

void UPBBossChargePattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	StartCharge();
}

void UPBBossChargePattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	ClearPatternTimers();
	DestroyChargeTelegraph();
	SetPinballMoveIgnored(false);
	SetPinballCollisionDamageBlocked(false);
	SetChargePatternState(EPBBossChargePatternState::None);
	TargetPinballActor = nullptr;
	ReboundedDistance = 0.0f;
	ChargeAimElapsedSeconds = 0.0f;
	ChargeAimDurationSeconds = 0.0f;
}

void UPBBossChargePattern::ExecuteNativePattern(APBBossBase* Boss)
{
	ExecutePattern_Implementation(Boss);
}

bool UPBBossChargePattern::PausePatternForExternalGroggy(APBBossBase* Boss)
{
	if (!Boss || ChargePatternState != EPBBossChargePatternState::Groggy)
	{
		return false;
	}

	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	return true;
}

bool UPBBossChargePattern::ResumePatternAfterExternalGroggy(APBBossBase* Boss)
{
	if (!Boss || ChargePatternState != EPBBossChargePatternState::Groggy)
	{
		return false;
	}

	FinishGroggy();
	return true;
}

void UPBBossChargePattern::SetChargePatternState(EPBBossChargePatternState NewState)
{
	ChargePatternState = NewState;
}

void UPBBossChargePattern::StartAiming(APBBossBase* Boss)
{
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossChargePatternState::Aiming);
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
		&UPBBossChargePattern::FinishAiming,
		TelegraphDurationSeconds,
		false);
}

void UPBBossChargePattern::FinishAiming()
{
	ClearChargeAimTimers();
	ClearChargeTelegraphTimer();
	DestroyChargeTelegraph();
	SetChargePatternState(EPBBossChargePatternState::Charging);

	if (APBBossBase* Boss = GetOwnerBoss())
	{
		ExecutePattern_Implementation(Boss);
		return;
	}

	SetChargePatternState(EPBBossChargePatternState::None);
	FinishPattern();
}

float UPBBossChargePattern::GetChargeTelegraphDurationSeconds() const
{
	return TelegraphDataList.IsEmpty()
		? 0.0f
		: TelegraphDataList[0].DurationSeconds;
}

void UPBBossChargePattern::PrepareCharge(APBBossBase* Boss)
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

void UPBBossChargePattern::RefreshChargeDirection(APBBossBase* Boss)
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

void UPBBossChargePattern::SpawnChargeTelegraph(APBBossBase* Boss)
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

void UPBBossChargePattern::StartChargeAim(float AimDurationSeconds)
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
		&UPBBossChargePattern::UpdateChargeAim,
		UpdateIntervalSeconds,
		true);

	Boss->GetWorldTimerManager().SetTimer(
		ChargeAimFinishTimerHandle,
		this,
		&UPBBossChargePattern::FinishChargeAim,
		AimDurationSeconds,
		false);
}

void UPBBossChargePattern::UpdateChargeAim()
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

void UPBBossChargePattern::FinishChargeAim()
{
	ChargeAimElapsedSeconds = ChargeAimDurationSeconds;
	UpdateChargeAim();
	ClearChargeAimTimers();
}

void UPBBossChargePattern::ClearChargeAimTimers()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(ChargeAimTimerHandle);
		Boss->GetWorldTimerManager().ClearTimer(ChargeAimFinishTimerHandle);
	}
}

void UPBBossChargePattern::UpdateChargeTelegraph() const
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

void UPBBossChargePattern::StartCharge()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossChargePatternState::Charging);
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
		&UPBBossChargePattern::UpdateCharge,
		UpdateIntervalSeconds,
		true);
}

void UPBBossChargePattern::UpdateCharge()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
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

void UPBBossChargePattern::HandleChargeBlocked(const FHitResult& Hit)
{
	static_cast<void>(Hit);

	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ChargeTimerHandle);
	StartRebound();
}

void UPBBossChargePattern::StartRebound()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossChargePatternState::Rebounding);
	ReboundedDistance = 0.0f;
	if (ReboundDistance <= 0.0f || ReboundSeconds <= 0.0f)
	{
		FinishRebound();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		ReboundTimerHandle,
		this,
		&UPBBossChargePattern::UpdateRebound,
		UpdateIntervalSeconds,
		true);
}

void UPBBossChargePattern::UpdateRebound()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
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

void UPBBossChargePattern::FinishRebound()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ReboundTimerHandle);
	StartGroggy();
}

void UPBBossChargePattern::StartGroggy()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossChargePatternState::Groggy);
	SetPinballCollisionDamageBlocked(false);
	if (GroggySeconds <= 0.0f)
	{
		FinishGroggy();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&UPBBossChargePattern::FinishGroggy,
		GroggySeconds,
		false);
}

void UPBBossChargePattern::FinishGroggy()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	StartReturn();
}

void UPBBossChargePattern::StartReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	SetChargePatternState(EPBBossChargePatternState::Returning);
	SetPinballCollisionDamageBlocked(true);

	if (ReturnSpeed <= 0.0f)
	{
		FinishReturn();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		ReturnTimerHandle,
		this,
		&UPBBossChargePattern::UpdateReturn,
		UpdateIntervalSeconds,
		true);
}

void UPBBossChargePattern::UpdateReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
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

void UPBBossChargePattern::FinishReturn()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		SetChargePatternState(EPBBossChargePatternState::None);
		FinishPattern();
		return;
	}

	Boss->GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
	SetPinballMoveIgnored(false);
	Boss->SetActorLocation(ChargeStartLocation, false);
	Boss->SetActorRotation(ChargeStartRotation);
	SetPinballCollisionDamageBlocked(false);
	SetChargePatternState(EPBBossChargePatternState::None);
	TargetPinballActor = nullptr;
	FinishPattern();
}

bool UPBBossChargePattern::MoveBossWithFloorIgnored(APBBossBase* Boss, const FVector& MoveOffset, FHitResult& OutHit) const
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

bool UPBBossChargePattern::IsFloorHit(const FHitResult& Hit) const
{
	if (!Hit.bBlockingHit)
	{
		return false;
	}

	const AActor* HitActor = Hit.GetActor();
	const FString HitActorName = IsValid(HitActor) ? HitActor->GetName() : FString();
	return Hit.ImpactNormal.Z > 0.5f || HitActorName.Contains(TEXT("Floor"));
}

void UPBBossChargePattern::ClearPatternTimers()
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

void UPBBossChargePattern::ClearChargeTelegraphTimer()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().ClearTimer(ChargeTelegraphTimerHandle);
	}
}

void UPBBossChargePattern::DestroyChargeTelegraph()
{
	if (!IsValid(SpawnedChargeTelegraph))
	{
		SpawnedChargeTelegraph = nullptr;
		return;
	}

	SpawnedChargeTelegraph->DestroyTelegraph();
	SpawnedChargeTelegraph = nullptr;
}

void UPBBossChargePattern::SetPinballMoveIgnored(bool IsIgnored) const
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

void UPBBossChargePattern::SetPinballCollisionDamageBlocked(bool IsBlocked) const
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->SetPinballCollisionDamageBlocked(IsBlocked);
	}
}
