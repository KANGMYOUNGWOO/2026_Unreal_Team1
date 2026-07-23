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
	Returning
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeMaxDistance = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReboundSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.0", Units = "cm"))
	float ReturnUpwardOffset = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.0", Units = "cm"))
	float UndergroundDepth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.0", Units = "cm"))
	float ReturnCurveControlDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.0", Units = "cm"))
	float EmergenceSearchRadius = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.0", Units = "cm"))
	float MinimumEmergenceDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.001"))
	float UpdateIntervalSeconds = 0.016f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge|Hit", meta = (ClampMin = "0.0"))
	float ChargeHitRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge|Hit", meta = (ClampMin = "0"))
	int32 ChargeDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge|Hit", meta = (ClampMin = "0.0"))
	float ChargeBounceVelocity = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge|Hit|Debug")
	bool IsDrawChargeHitRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Charge", meta = (AllowPrivateAccess = "true"))
	EPBBossSnakeChargePatternState ChargePatternState = EPBBossSnakeChargePatternState::None;

private:
	void SetChargePatternState(EPBBossSnakeChargePatternState NewState);
	void StartAiming(APBBossBase* Boss);
	UFUNCTION()
	void HandleChargeTelegraphFinished(FVector TargetLocation, FVector Direction);
	float GetChargeTelegraphDurationSeconds() const;
	void PrepareCharge(APBBossBase* Boss);
	void RefreshChargeDirection(APBBossBase* Boss);
	void BuildChargePath(const FVector& TargetLocation);
	FVector GetChargePathLocation(float Distance) const;
	void SpawnChargeTelegraph(APBBossBase* Boss);
	void StartCharge();
	void UpdateCharge();
	void MoveBossByChargeDistance(APBBossBase* Boss, float ChargeDistance);
	void FinishCharge();
	void StartReturn();
	FVector FindEmergenceLocation(APBBossBase* Boss) const;
	void UpdateReturn();
	void FinishReturn();
	void ClearPatternTimers();
	void DestroyChargeTelegraph();
	void SetPinballCollisionDamageBlocked(bool IsBlocked) const;
	void CreateChargeHitCollision(APBBossBase* Boss);
	void DestroyChargeHitCollision();
	void ApplyChargeHit(class APBBallBase* Ball);

	UFUNCTION()
	void HandleChargeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	FVector ChargeStartLocation = FVector::ZeroVector;
	FVector ChargeEndLocation = FVector::ZeroVector;
	FVector ReturnEndLocation = FVector::ZeroVector;
	FVector ReturnFirstControlLocation = FVector::ZeroVector;
	FVector ReturnSecondControlLocation = FVector::ZeroVector;
	FRotator ChargeStartRotation = FRotator::ZeroRotator;
	FVector ChargeDirection = FVector::ForwardVector;
	float ChargePathLength = 0.0f;
	float ChargeProgressDistance = 0.0f;
	float ChargeAimElapsedSeconds = 0.0f;
	float ChargeAimDurationSeconds = 0.0f;
	float ReturnElapsedSeconds = 0.0f;
	bool IsChargeMovementStarted = false;

	UPROPERTY(Transient)
	TObjectPtr<class APBBossChargeTelegraph> SpawnedChargeTelegraph;

	UPROPERTY(Transient)
	TObjectPtr<class USphereComponent> ChargeHitCollision;

	TSet<TObjectKey<class APBBallBase>> DamagedBalls;
	FTimerHandle ChargeTimerHandle;
	FTimerHandle ReturnTimerHandle;
};
