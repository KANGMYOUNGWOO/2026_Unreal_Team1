// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../IShopPurchaseHandler.h"
#include "PBShopManager.generated.h"

class UUPBShopViewModel;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBShopManager : public UObject , public IIShopPurchaseHandler
{
	GENERATED_BODY()
	
public :
	
	
	
	TArray<int32> OpenShop();
	
	bool BuyItem(int32 SlotIndex) override;
	
    int32 GetCurrentGold() const;
	
private :
	
	int32 CurrentGold;
	TArray<int32> CurrentShopItemBallIds;
	TArray<bool> ShopItemIsSell;
	
};
