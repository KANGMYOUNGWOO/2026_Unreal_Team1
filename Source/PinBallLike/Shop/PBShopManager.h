// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include  "../Interface/IShopActorHandler.h"
#include  "../Interface/IShopPurchaseHandler.h"
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
	
	TArray<FName> OpenShop();
	
	bool BuyItem(int32 SlotIndex) override;
	void RequestPurchase(int32 SlotIndex) override;
	
    int32 GetCurrentGold() const;
	
	void SetShopActorHandler(IIShopActorHandler* Handler);
	
	bool RerollShop();
	
	
	int32 GetShopItemPrice(int32 SlotIndex) const;
	FName GetShopItemRowName(int32 SlotIndex) const;
	const TArray<FName>& GetCurrentShopItemBallIds() const;
	bool IsShopItemSold(int32 SlotIndex) const;
	void ApplyShopPriceDiscount(FName ModifyType, float Value);
	void ApplyShopRerollDiscount(FName ModifyType, float Value);
	void RefreshActiveSynergyDiscounts();
private :
	
	int32 CurrentGold;
	TArray<FName> CurrentShopItemBallIds;
	TArray<bool> ShopItemIsSell;
private:
	// ShopTable에서 상품을 무작위로 뽑아 현재 상점 목록을 재구성한다.
	bool GenerateShopItems(int32 SlotCount);
	IIShopActorHandler* ShopActorHandler;
	UPROPERTY()
	TArray<bool> ShopItemIsSold;
	int RerollChance;
	UPROPERTY()
	TArray<FName> CurrentShopItemRowNames;
	int32 BaseRerollCost = 30;
	int32 CurrentRerollCost = 30;
	float ShopPriceDiscountPercent = 0.0f;
	float ShopPriceDiscountAmount = 0.0f;
	float ShopRerollDiscountPercent = 0.0f;
	float ShopRerollDiscountAmount = 0.0f;
	
	int32 CalculateDiscountedPrice(int32 BasePrice, float DiscountAmount, float DiscountPercent) const;
	
};
