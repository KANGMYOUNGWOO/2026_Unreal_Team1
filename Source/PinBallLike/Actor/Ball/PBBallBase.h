#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBallBase.generated.h"

class UPBBaseStatComponent;
class UPBBaseResourceComponent;
class UPBBallComboComponent;
class UPBBallHitReactionComponent;
class UPBBallPhysicsComponent;
class USphereComponent;

USTRUCT(BlueprintType)
struct FPBBallStatData
{
	GENERATED_BODY()

	FPBBallStatData() = default;
	FPBBallStatData(FName InStatName, int32 InValue)
		: StatName(InStatName), Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Stat")
	FName StatName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Stat")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FPBBallResourceData
{
	GENERATED_BODY()

	FPBBallResourceData() = default;
	FPBBallResourceData(FName InResourceName, float InCurrent, float InMax, float InRegenPerSecond)
		: ResourceName(InResourceName), Current(InCurrent), Max(InMax), RegenPerSecond(InRegenPerSecond)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Resource")
	FName ResourceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Resource")
	float Current = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Resource")
	float Max = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Resource")
	float RegenPerSecond = 0.0f;
};

UCLASS()
class PINBALLLIKE_API APBBallBase : public AActor
#include "PinBallLike/Interface/BallGauge.h"
#include "PinBallLike/Interface/BallStat.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Damagable.h"
#include "PBBallBase.generated.h"

class UPBBallStatComponent;
class UPBBallGaugeComponent;
class UPBBallComboComponent;
class USphereComponent;
class UPBBallMovementComponent;

UCLASS()
class PINBALLLIKE_API APBBallBase : public AActor, public IBallStat, public IBallGauge, public IComboable, public IDamagable
{
	GENERATED_BODY()

public:
	APBBallBase();

	UFUNCTION(BlueprintCallable, Category = "Ball|Stat")
	void ApplyStatData(const TArray<FPBBallStatData>& StatData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Resource")
	void ApplyResourceData(const TArray<FPBBallResourceData>& ResourceData);
	
protected:
	virtual void BeginPlay() override;
	
private:
	void InitializeDefaultStats();
	void InitializeDefaultResources();

	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Ball|Stat")
	TObjectPtr<UPBBaseStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Resource")
	TObjectPtr<UPBBaseResourceComponent> ResourceComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	TObjectPtr<UPBBallComboComponent> ComboComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<UPBBallHitReactionComponent> HitReactionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Ball|Stat")
	TArray<FPBBallStatData> DefaultStats;

	UPROPERTY(EditDefaultsOnly, Category = "Ball|Resource")
	TArray<FPBBallResourceData> DefaultResources;

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
	TObjectPtr<UPBBallMovementComponent> MovementComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Ball|Stat")
	TObjectPtr<UPBBallStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Gauge")
	TObjectPtr<UPBBallGaugeComponent> GaugeComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	TObjectPtr<UPBBallComboComponent> ComboComponent;
	
	FString DisplayName;
};
