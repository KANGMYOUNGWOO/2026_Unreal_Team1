// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopPresenter.h"
#include  "../PBShopManager.h"
#include "PBShopViewModel.h"
#include "../DisplayActor/PBShopDisplayActor.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/PBShopWidget.h"


void UPBShopPresenter::SetUp(UPBShopManager* InShopManager, UPBShopViewModel* InViewModel,APBShopDisplayActor* InDisplayActor)
{
	ShopManager = InShopManager;
	ViewModel = InViewModel;
	ShopDisplayActor = InDisplayActor;
	
	RefreshViewModel();
}

void UPBShopPresenter::OpenShop()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
	
	if(!PC)
	{
		return;
	}
	
	if(!ViewModel)
	{
		return;
	}
	
	if (!ShopManager)
	{
		return;
	}
	
	if(!ShopWidget)
	{
		if (!ShopWidgetClass)
		{
			return;
		}
		
		ShopWidget = CreateWidget<UPBShopWidget>(PC, ShopWidgetClass);
		if (!ShopWidget)
		{
			return;
		}
		ShopWidget->AddToViewport();
		
	}
	
	TArray<int32> ShopItemsId = ShopManager->OpenShop();
	if (ShopDisplayActor)
	{
		ShopDisplayActor->DisplayItems(ShopItemsId);
		ShopWidget->SetShopSlotWorldLocations(
			ShopDisplayActor->GetSlotWorldLocation());
	}
	
	RefreshViewModel();
	
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeGameAndUI());
}

void UPBShopPresenter::CloseShop()
{
	if (ShopWidget)
	{
		ShopWidget->RemoveFromParent();
		ShopWidget = nullptr;
	}
	
	if (ShopDisplayActor)
	{
		//ShopDisplayActor->Clear
	}
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		
	}
}

bool UPBShopPresenter::BuyItem(int32 SlotIndex)
{
	if (!ShopManager)
	{
		return false;
	}
	
	const bool IsSuccess = ShopManager -> BuyItem(SlotIndex);
	
	RefreshViewModel();

	return IsSuccess;
}

void UPBShopPresenter::RefreshViewModel()
{
	if (!ShopManager || !ViewModel)
	{
		return;
	}

	ViewModel->SetGold(ShopManager->GetCurrentGold());
}
