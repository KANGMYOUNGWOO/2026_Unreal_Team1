// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBShopActor.generated.h"

class UPBShopViewModel;
class UPBShopManager;
class APBShopDisplayActor;
class UPBShopWidget;

UCLASS()
class PINBALLLIKE_API APBShopActor : public AActor
{
	GENERATED_BODY()
	
	void SetUp(UPBShopManager* InShopManager, UPBShopViewModel* InViewModel , APBShopDisplayActor* InDisplayActor);
	
	UFUNCTION(BlueprintCallable)
	void OpenShop();
	
	UFUNCTION(BlueprintCallable)
	void CloseShop();
	
	bool BuyItem(int32 SlotIndex);
	
private :
	
	void RefreshViewModel();
	
private:
	
	UPROPERTY()
	TObjectPtr<UPBShopManager> ShopManager;
	
	UPROPERTY()
	TObjectPtr<UPBShopViewModel> ViewModel;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TObjectPtr<APBShopDisplayActor> ShopDisplayActor;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TSubclassOf<UPBShopWidget> ShopWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UPBShopWidget> ShopWidget;

};
