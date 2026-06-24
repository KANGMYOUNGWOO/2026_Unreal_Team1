// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/ResourceProvider.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PBBallBase.generated.h"

class UPBBaseStatComponent;
class UPBBaseResourceComponent;
class UPBBallComboComponent;
class UPBBallCollisionComponent;
class USphereComponent;
class UPBBallPhysicsComponent;

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
class PINBALLLIKE_API APBBallBase : public AActor, public IStatProvider, public IResourceProvider, public IComboable, public IMovable, public IDamageable
{
	GENERATED_BODY()

public:
	APBBallBase();

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void LaunchBall(FVector Impulse);

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void AddVelocity(FVector VelocityToAdd);

	UFUNCTION(BlueprintCallable, Category = "Ball|Stat")
	void ApplyStatData(const TArray<FPBBallStatData>& StatData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Resource")
	void ApplyResourceData(const TArray<FPBBallResourceData>& ResourceData);

	virtual bool HasStat(FName StatName) const override;
	virtual int32 GetStat(FName StatName) const override;
	virtual void SetStat(FName StatName, int32 Value) override;
	virtual void ApplyStat(FName StatName, int32 Delta) override;

	virtual bool HasResource(FName ResourceName) const override;
	virtual float GetResourceCurrent(FName ResourceName) const override;
	virtual float GetResourceMax(FName ResourceName) const override;
	virtual float GetResourceRatio(FName ResourceName) const override;
	virtual void SetResource(FName ResourceName, float Current, float Max) override;
	virtual void SetResourceCurrent(FName ResourceName, float Value) override;
	virtual void SetResourceMax(FName ResourceName, float Value, bool bFillCurrent) override;
	virtual void SetResourceRegenPerSecond(FName ResourceName, float Value) override;
	virtual void ApplyResourceDelta(FName ResourceName, float Delta) override;
	virtual bool CanConsumeResource(FName ResourceName, float Cost) const override;
	virtual bool ConsumeResource(FName ResourceName, float Cost) override;

	virtual int32 GetCombo() const override;
	virtual void SetCombo(int32 Value) override;
	virtual void AddCombo(int32 Delta) override;
	virtual bool TryConsumeCombo(int32 Cost) override;
	virtual void ResetCombo() override;

	virtual FVector GetVelocity() const override;
	virtual void AddImpulse(FVector Impulse) override;
	virtual void StopMovement() override;
	virtual void PauseMovement() override;
	virtual void ResumeMovement() override;
	virtual bool IsMovementPaused() const override;

	virtual void TakeDamage(int32 Damage) override;
	virtual bool IsDead() const override;
	
private:
	void InitializeDefaultStats();
	void InitializeDefaultResources();
	void ApplyStatToComponents(FName StatName);

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
	TObjectPtr<UPBBallCollisionComponent> CollisionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Ball|Stat")
	TArray<FPBBallStatData> DefaultStats;

	UPROPERTY(EditDefaultsOnly, Category = "Ball|Resource")
	TArray<FPBBallResourceData> DefaultResources;
	
	FString DisplayName;
};
