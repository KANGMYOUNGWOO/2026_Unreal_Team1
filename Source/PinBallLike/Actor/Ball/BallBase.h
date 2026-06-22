
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/BallGauge.h"
#include "PinBallLike/Interface/BallStat.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Damagable.h"
#include "BallBase.generated.h"

class UPBBallStatComponent;
class UPBBallGaugeComponent;
class UPBBallComboComponent;
class USphereComponent;
class UPinballBallMovementComponent;

UCLASS()
class PINBALLLIKE_API ABallBase : public AActor, public IBallStat, public IBallGauge, public IComboable, public IDamagable
{
	GENERATED_BODY()

public:
	ABallBase();

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void LaunchBall(FVector Impulse);

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void AddVelocity(FVector VelocityToAdd);

	virtual int32 GetStat(EBallStatType Type) const override;
	virtual void ApplyStat(EBallStatType Type, int32 Delta) override;

	virtual bool HasGauge(EBallGaugeType Type) const override;
	virtual float GetGaugeCurrent(EBallGaugeType Type) const override;
	virtual float GetGaugeMax(EBallGaugeType Type) const override;
	virtual float GetGaugeRatio(EBallGaugeType Type) const override;
	virtual void SetGauge(EBallGaugeType Type, float Current, float Max) override;
	virtual void SetGaugeCurrent(EBallGaugeType Type, float Value) override;
	virtual void SetGaugeMax(EBallGaugeType Type, float Value, bool bFillCurrent) override;
	virtual void SetGaugeRegenPerSecond(EBallGaugeType Type, float Value) override;
	virtual void ApplyGaugeDelta(EBallGaugeType Type, float Delta) override;
	virtual bool CanConsumeGauge(EBallGaugeType Type, float Cost) const override;
	virtual bool ConsumeGauge(EBallGaugeType Type, float Cost) override;

	virtual int32 GetCombo() const override;
	virtual void SetCombo(int32 Value) override;
	virtual void AddCombo(int32 Delta) override;
	virtual bool TryConsumeCombo(int32 Cost) override;
	virtual void ResetCombo() override;

	virtual void TakeDamage(int32 Damage) override;
	virtual bool IsDead() const override;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPinballBallMovementComponent> MovementComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Ball|Stat")
	TObjectPtr<UPBBallStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Gauge")
	TObjectPtr<UPBBallGaugeComponent> GaugeComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	TObjectPtr<UPBBallComboComponent> ComboComponent;
	
	FString DisplayName;
};
