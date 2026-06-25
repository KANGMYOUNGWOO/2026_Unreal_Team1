// Fill out your copyright notice in the Description page of Project Settings.

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
	
	FString DisplayName;
};
