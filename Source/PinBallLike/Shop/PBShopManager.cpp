// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopManager.h"
#include "../Subsystem/BallDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"


TArray<int32> UPBShopManager::OpenShop()
{
	
	CurrentShopItemBallIds.Empty();
	CurrentGold = 1000;
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
	ShopItemIsSell.Init(false, 8);
	
	return  CurrentShopItemBallIds;
	
}

bool UPBShopManager::BuyItem(int32 SlotIndex)
{

	
	if (ShopItemIsSell[SlotIndex])
	{
		return false;
	}
	
	UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
	if (!GI)
	{
		return false;
	}
	
	UBallDataSubsystem* DataSub = GI->GetSubsystem<UBallDataSubsystem>();	
	
	if (!DataSub)
	{
		return false;
	}
	
	if (SlotIndex >= CurrentShopItemBallIds.Num()) { return false; }
	
	int32 SelectedBallId = CurrentShopItemBallIds[SlotIndex];
	
	const FBallDataStruct* BallData = DataSub->GetBallData(SelectedBallId);

	if (!BallData) { return false; }
	
	if (CurrentGold >= BallData->BallPrice)
	{
		
		ShopItemIsSell[SlotIndex] = true;
		CurrentGold -= BallData->BallPrice;
		UE_LOG(LogTemp, Warning, TEXT("Current Gold : %d"), CurrentGold);
		return true;	
	}
	else
	{
		return false;
	}
}

int32 UPBShopManager::GetCurrentGold() const
{
	return CurrentGold;
}


