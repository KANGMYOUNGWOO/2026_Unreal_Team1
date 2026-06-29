// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../IShopPurchaseHandler.h"
#include "../../IShopActorHandler.h"
#include "PBShopActor.generated.h"

class UPBShopViewModel;
class UPBShopManager;
class APBShopDisplayActor;
class UPBShopWidget;

UCLASS()
class PINBALLLIKE_API APBShopActor : public AActor , public IIShopActorHandler
{
	GENERATED_BODY()
	
	
public:
	APBShopActor();
	
	
	
	UFUNCTION(BlueprintCallable)
	void OpenShop();
	
	UFUNCTION(BlueprintCallable)
	void CloseShop();
	
	void BuyItem(int32 SlotIndex) override;
	
private :
	
	void RefreshViewModel();

	
private:
	
	UPROPERTY()
	TObjectPtr<UPBShopManager> ShopManager;
	
	IIShopPurchaseHandler *ShopPurchaseHandler;
	
	UPROPERTY()
	TObjectPtr<UPBShopViewModel> ShopViewModel;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TObjectPtr<APBShopDisplayActor> ShopDisplayActor;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TSubclassOf<UPBShopWidget> ShopWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UPBShopWidget> ShopWidget;
	
	

protected:
	virtual void BeginPlay() override;
	
};
