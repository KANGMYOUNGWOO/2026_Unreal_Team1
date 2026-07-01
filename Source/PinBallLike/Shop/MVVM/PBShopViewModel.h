// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBShopViewModel.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBShopViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Shop|ViewModel")
	void SetGold(int32 NewGold);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Shop|ViewModel")
	FText GoldText = FText::AsNumber(0);

private:
	UPROPERTY()
	int32 Gold = 0;
};
