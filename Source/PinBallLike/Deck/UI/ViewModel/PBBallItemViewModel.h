// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Ball/PBBallItemViewData.h"
#include "PBBallItemViewModel.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBallItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BallDeck|ItemViewModel")
	void SetBallItemViewData(const FPBBallItemViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "BallDeck|ItemViewModel")
	void ClearBall();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	bool bHasBall = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	int32 StarLevel = 1;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|ItemViewModel")
	float Progress = 1.0f;

private:
	float CalculateStarProgress(int32 InStarLevel) const;
};
