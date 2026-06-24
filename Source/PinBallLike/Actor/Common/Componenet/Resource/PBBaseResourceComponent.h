// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBaseResourceComponent.generated.h"

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
class PINBALLLIKE_API UPBBaseResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBaseResourceComponent();

	UFUNCTION(BlueprintCallable, Category="Resource")
	bool HasResource(FName ResourceName) const;

	UFUNCTION(BlueprintCallable, Category="Resource")
	float GetCurrent(FName ResourceName) const;

	UFUNCTION(BlueprintCallable, Category="Resource")
	float GetMax(FName ResourceName) const;

	UFUNCTION(BlueprintCallable, Category="Resource")
	float GetRatio(FName ResourceName) const;

	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetResource(FName ResourceName, float Current, float Max);

	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetCurrent(FName ResourceName, float Value);

	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetMax(FName ResourceName, float Value, bool bFillCurrent);

	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetRegenPerSecond(FName ResourceName, float Value);

	UFUNCTION(BlueprintCallable, Category="Resource")
	void ApplyDelta(FName ResourceName, float Delta);

	UFUNCTION(BlueprintCallable, Category="Resource")
	bool CanConsume(FName ResourceName, float Cost) const;

	UFUNCTION(BlueprintCallable, Category="Resource")
	bool Consume(FName ResourceName, float Cost);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource", meta=(AllowPrivateAccess="true"))
	TMap<FName, FPBResourceState> Resources;

private:
	FPBResourceState* FindResource(FName ResourceName);
	const FPBResourceState* FindResource(FName ResourceName) const;
	void BroadcastResourceChanged(FName ResourceName, const FPBResourceState& Resource);
};
