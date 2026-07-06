// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBBallItemViewModel.generated.h"

class APBBallBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBallItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BallDeck|ItemViewModel")
	void SetBall(APBBallBase* InBall);

	UFUNCTION(BlueprintCallable, Category = "BallDeck|ItemViewModel")
	void ClearBall();

	UFUNCTION(BlueprintCallable, Category = "BallDeck|ItemViewModel")
	void RefreshFromBall();

	UFUNCTION(BlueprintPure, Category = "BallDeck|ItemViewModel")
	APBBallBase* GetBall() const { return Ball; }

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	bool bHasBall = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	FText BallNameText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	FText BallDescriptionText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	float EnergyPercent = 0.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> Ball = nullptr;
};
