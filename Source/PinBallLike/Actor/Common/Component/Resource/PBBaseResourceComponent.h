// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/ResourceProvider.h"
#include "PBBaseResourceComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FPBOnResourceCurrentChanged, FName, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FPBOnResourceChanged, FName);

USTRUCT(BlueprintType)
struct FPBResourceState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 CurrentRaw = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxRaw = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 RegenPerSecondRaw = 0;

	double RegenRawRemainder = 0.0;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBaseResourceComponent : public UActorComponent, public IResourceProvider, public IDamageable
{
	GENERATED_BODY()

public:
	UPBBaseResourceComponent();

	FPBOnResourceCurrentChanged OnResourceCurrentChanged;
	FPBOnResourceChanged OnResourceChanged;
	
	UFUNCTION(BlueprintCallable, Category="Resource")
	virtual bool HasResource(FName ResourceName) const override;
	UFUNCTION(BlueprintCallable, Category="Resource")
	virtual float GetResourceCurrent(FName ResourceName) const override;
	UFUNCTION(BlueprintCallable, Category="Resource")
	virtual float GetResourceMax(FName ResourceName) const override;
	UFUNCTION(BlueprintCallable, Category="Resource")
	virtual float GetResourceRatio(FName ResourceName) const override;
	
	UFUNCTION(BlueprintCallable, Category="Resource")
	virtual void TakeDamage(int32 Damage) override;
	UFUNCTION(BlueprintCallable, Category="Resource")
	virtual bool IsDead() const override;

	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetResource(FName ResourceName, float Current, float Max);
	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetResourceCurrent(FName ResourceName, float Value);
	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetResourceMax(FName ResourceName, float Value, bool bFillCurrent);
	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetResourceRegenPerSecond(FName ResourceName, float Value);

	UFUNCTION(BlueprintCallable, Category="Resource")
	void ApplyResourceDelta(FName ResourceName, float Delta);
	UFUNCTION(BlueprintCallable, Category="Resource")
	bool CanConsumeResource(FName ResourceName, float Cost) const;
	UFUNCTION(BlueprintCallable, Category="Resource")
	bool ConsumeResource(FName ResourceName, float Cost);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	float GetCurrent(FName ResourceName) const;
	float GetMax(FName ResourceName) const;
	float GetRatio(FName ResourceName) const;
	
private:
	FPBResourceState* FindResource(FName ResourceName);
	const FPBResourceState* FindResource(FName ResourceName) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource", meta=(AllowPrivateAccess="true"))
	TMap<FName, FPBResourceState> Resources;
};