// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBShopViewModel.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBShopViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public :
	
	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetGold() const;
	
	void SetGold(int32 NewGold);
	
private:
	UPROPERTY()
	int32 Gold;
	
};
