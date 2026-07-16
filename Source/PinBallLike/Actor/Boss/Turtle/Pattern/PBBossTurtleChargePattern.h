#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBBossTurtleChargePattern.generated.h"

class APBBallBase;
class APBTurtleBoss;
class UAnimSequence;
class UPrimitiveComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossTurtleChargePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	UPBBossTurtleChargePattern();

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UFUNCTION()
	void HandleChargeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	void UpdateCharge();
	void StartCharge();
	void StopCharge();
	void FinishCharge();
	void UpdateReturnToMoveArea();
	void CompleteChargePattern();
	void ApplyChargeDamage(APBBallBase* Ball);
	void CleanupCharge();
	bool FindChargeEndLocation(const APBTurtleBoss* Boss, FVector& OutChargeEndLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge")
	TObjectPtr<UAnimSequence> ChargeAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0.0"))
	float ChargeSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0"))
	int32 ChargeDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0.0"))
	float ChargeCollisionRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge")
	float ChargeFacingYawOffset = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge|Boundary")
	FName ChargeBoundaryActorName = TEXT("TriggerBox");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge|Boundary", meta = (ClampMin = "0.0"))
	float ChargeBoundaryPadding = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0.0"))
	float ChargeStartTimeSeconds = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0.0"))
	float ChargeEndTimeSeconds = 4.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0.001"))
	float UpdateIntervalSeconds = 0.016f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge", meta = (ClampMin = "0.0"))
	float ReturnAcceptanceRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge|Debug")
	bool IsDrawChargeDamageRange = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Charge|Debug", meta = (ClampMin = "0.0"))
	float ChargeDamageRangeLineThickness = 5.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> ChargeCollision;

	TWeakObjectPtr<APBTurtleBoss> TurtleBoss;
	TSet<TObjectKey<APBBallBase>> DamagedBalls;
	FVector ChargeDirection = FVector::ForwardVector;
	FVector ChargeEndLocation = FVector::ZeroVector;
	FVector ReturnTargetLocation = FVector::ZeroVector;
	bool IsChargeEndLocationValid = false;
	FTimerHandle ChargeStartTimerHandle;
	FTimerHandle ChargeUpdateTimerHandle;
	FTimerHandle ChargeStopTimerHandle;
	FTimerHandle ChargeFinishTimerHandle;
	FTimerHandle ReturnUpdateTimerHandle;
};
