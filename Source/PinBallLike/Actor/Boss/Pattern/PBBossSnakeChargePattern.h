#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternBase.h"
#include "TimerManager.h"
#include "PBBossSnakeChargePattern.generated.h"

UENUM(BlueprintType)
enum class EPBBossSnakeChargePatternState : uint8
{
	None,
	Aiming,
	Charging,
	Rebounding,
	Groggy
};

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossSnakeChargePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;
	virtual void ExecuteNativePattern(APBBossBase* Boss) override;
	virtual bool PausePatternForExternalGroggy(APBBossBase* Boss) override;
	virtual bool ResumePatternAfterExternalGroggy(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeMaxDistance = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReboundSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float GroggySeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.001"))
	float UpdateIntervalSeconds = 0.016f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Charge", meta = (AllowPrivateAccess = "true"))
	EPBBossSnakeChargePatternState ChargePatternState = EPBBossSnakeChargePatternState::None;

private:
	void SetChargePatternState(EPBBossSnakeChargePatternState NewState);
	void StartAiming(APBBossBase* Boss);
	void FinishAiming();
	float GetChargeTelegraphDurationSeconds() const;
	void PrepareCharge(APBBossBase* Boss);
	void RefreshChargeDirection(APBBossBase* Boss);
	void ApplySnakeChargePose(APBBossBase* Boss, float Alpha) const;
	void SpawnChargeTelegraph(APBBossBase* Boss);
	void StartChargeAim(float AimDurationSeconds);
	void UpdateChargeAim();
	void FinishChargeAim();
	void ClearChargeAimTimers();
	void UpdateChargeTelegraph() const;
	void StartCharge();
	void UpdateCharge();
	void HandleChargeBlocked(const FHitResult& Hit);
	void StartRebound();
	void UpdateRebound();
	void FinishRebound();
	void StartGroggy();
	void FinishGroggy();
	void ClearPatternTimers();
	void ClearChargeTelegraphTimer();
	void DestroyChargeTelegraph();
	void SetPinballCollisionDamageBlocked(bool IsBlocked) const;

	FVector ChargeStartLocation = FVector::ZeroVector;
	FVector ChargeDirection = FVector::ForwardVector;
	float ChargeProgressDistance = 0.0f;
	float ReboundProgressAlpha = 0.0f;
	float ChargeAimElapsedSeconds = 0.0f;
	float ChargeAimDurationSeconds = 0.0f;
	float GroggyEndTimeSeconds = 0.0f;
	float PausedGroggyRemainingSeconds = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<class APBBossChargeTelegraph> SpawnedChargeTelegraph;

	FTimerHandle ChargeAimTimerHandle;
	FTimerHandle ChargeAimFinishTimerHandle;
	FTimerHandle ChargeTelegraphTimerHandle;
	FTimerHandle ChargeTimerHandle;
	FTimerHandle ReboundTimerHandle;
	FTimerHandle GroggyTimerHandle;
};
