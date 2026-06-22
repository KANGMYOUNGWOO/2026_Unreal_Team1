// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/BallGauge.h"
#include "PBBallGaugeComponent.generated.h"


USTRUCT(BlueprintType)
struct FBallGaugeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gauge", meta = (ToolTip = "Scaled by 1000. 0.001 is stored as 1."))
	int32 CurrentRaw = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gauge", meta = (ToolTip = "Scaled by 1000. 0.001 is stored as 1."))
	int32 MaxRaw = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gauge", meta = (ToolTip = "Scaled by 1000. 0.001 per second is stored as 1."))
	int32 RegenPerSecondRaw = 0;

	double RegenRawRemainder = 0.0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBallGaugeChanged, EBallGaugeType, Type, float, Current, float, Max);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallGaugeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallGaugeComponent();

	UPROPERTY(BlueprintAssignable, Category = "Ball|Gauge")
	FOnBallGaugeChanged OnGaugeChanged;

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	bool HasGauge(EBallGaugeType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	float GetCurrent(EBallGaugeType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	float GetMax(EBallGaugeType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	float GetRatio(EBallGaugeType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	void SetGauge(EBallGaugeType Type, float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	void SetCurrent(EBallGaugeType Type, float Value);

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	void SetMax(EBallGaugeType Type, float Value, bool bFillCurrent);

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	void SetRegenPerSecond(EBallGaugeType Type, float Value);

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	void ApplyDelta(EBallGaugeType Type, float Delta);

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	bool CanConsume(EBallGaugeType Type, float Cost) const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Gauge")
	bool Consume(EBallGaugeType Type, float Cost);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, Category = "Ball|Gauge")
	TMap<EBallGaugeType, FBallGaugeState> Gauges;

	FBallGaugeState* FindGauge(EBallGaugeType Type);
	const FBallGaugeState* FindGauge(EBallGaugeType Type) const;
	void BroadcastGaugeChanged(EBallGaugeType Type, const FBallGaugeState& Gauge);
};
