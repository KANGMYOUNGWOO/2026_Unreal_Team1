// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interface/IShopActorHandler.h"
#include  "../../Interface/IShopPurchaseHandler.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "PinBallLike/Interface/PBChoiceNodeAction.h"
#include "PBShopActor.generated.h"

class UPBShopViewModel;
class UPBShopManager;
class APBShopDisplayActor;
class UPBShopWidget;

UCLASS()
class PINBALLLIKE_API APBShopActor : public AActor , public IIShopActorHandler , public IPBChoiceNodeAction
{
	GENERATED_BODY()
	
	
public:
	APBShopActor();
	
	
	
	UFUNCTION(BlueprintCallable)
	void OpenShop();
	
	UFUNCTION(BlueprintCallable)
	void CloseShop();
	
	void BuyItem(int32 SlotIndex) override;
	void ShoPPurchaseConfirm(int SlotIndex) override;
	void OpenAbility() override;
	void ShowNotEnoughGoldPopup() override;
	
	bool ApplyViewModelToWidget(UUserWidget* Widget);
	
private :
	
	void RefreshViewModel();
	

	
private:
	
	UPROPERTY()
	TObjectPtr<UPBShopManager> ShopManager;
	
	IIShopPurchaseHandler *ShopPurchaseHandler;
	
	UPROPERTY(Transient)
	TObjectPtr<UPBShopViewModel> ShopViewModel;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TObjectPtr<APBShopDisplayActor> ShopDisplayActor;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TSubclassOf<UPBShopWidget> ShopWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UPBShopWidget> ShopWidget;
	
	void HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message);

	FGameplayMessageListenerHandle ExitStartHandle;
	
private:
	bool bPendingUpdateSlotPositions = false;
	
protected:
	virtual void BeginPlay() override;
	
};
