// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBSellZoneViewModel.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBSellZoneViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Sell|ViewModel")
	void SetSellEnabled(bool bInSellEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sell|ViewModel")
	void SetSellBall(int32 InBallInstanceId, int32 InPrice);

	UFUNCTION(BlueprintCallable, Category = "Sell|ViewModel")
	void ClearSellBall();

	UFUNCTION(BlueprintPure, Category = "Sell|ViewModel")
	bool CanSell() const;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Sell|ViewModel")
	bool bSellEnabled = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Sell|ViewModel")
	int32 Price = 0;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Sell|ViewModel")
	FText PriceText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Sell|ViewModel")
	int32 BallInstanceId = INDEX_NONE;
};
