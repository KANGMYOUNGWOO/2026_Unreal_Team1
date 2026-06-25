// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopManager.h"
#include "../Subsystem/BallDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"


TArray<int32> UPBShopManager::OpenShop()
{
	
	CurrentShopItemBallIds.Empty();
	
	UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
	if (!GI)
	{
		return {};
	}
	
	UBallDataSubsystem* DataSub = GI->GetSubsystem<UBallDataSubsystem>();	
	
	if (!DataSub)
	{
		return{};
	}
	
	CurrentShopItemBallIds  = DataSub->GetRandomBalls(8);
	return  CurrentShopItemBallIds;
	
}

bool UPBShopManager::BuyItem(int32 SlotIndex)
{
	return false;
}

int32 UPBShopManager::GetCurrentGold() const
{
	return CurrentGold;
}


