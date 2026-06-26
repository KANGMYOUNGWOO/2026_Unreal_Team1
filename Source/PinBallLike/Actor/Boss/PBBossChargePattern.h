#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternBase.h"
#include "TimerManager.h"
#include "PBBossChargePattern.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossChargePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;
	virtual void ExecuteNativePattern(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeMaxDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReboundDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReboundSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float GroggySeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReturnSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReturnAcceptanceRadius = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.001"))
	float UpdateIntervalSeconds = 0.016f;

private:
	void PrepareCharge(APBBossBase* Boss);
	void SpawnChargeTelegraph(APBBossBase* Boss);
	void StartExecuteChargePattern();
	void StartCharge();
	void UpdateCharge();
	void HandleChargeBlocked(const FHitResult& Hit);
	void StartRebound();
	void FinishRebound();
	void StartGroggy();
	void FinishGroggy();
	void StartReturn();
	void UpdateReturn();
	void FinishReturn();
	bool MoveBossWithFloorIgnored(APBBossBase* Boss, const FVector& MoveOffset, FHitResult& OutHit) const;
	bool IsFloorHit(const FHitResult& Hit) const;
	void ClearPatternTimers();
	void ClearChargeTelegraphTimer();
	void DestroyChargeTelegraph();
	void SetPinballMoveIgnored(bool IsIgnored) const;
	void SetPinballCollisionDamageBlocked(bool IsBlocked) const;
	AActor* FindPinballActor() const;

	FVector ChargeStartLocation = FVector::ZeroVector;
	FRotator ChargeStartRotation = FRotator::ZeroRotator;
	FVector ChargeDirection = FVector::ForwardVector;
	float ChargedDistance = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetPinballActor;

	UPROPERTY(Transient)
	TObjectPtr<class APBBossChargeTelegraph> SpawnedChargeTelegraph;

	FTimerHandle ChargeTelegraphTimerHandle;
	FTimerHandle ChargeTimerHandle;
	FTimerHandle ReboundTimerHandle;
	FTimerHandle GroggyTimerHandle;
	FTimerHandle ReturnTimerHandle;
};
