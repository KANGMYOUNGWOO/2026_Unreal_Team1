// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopActor.h"
#include  "../PBShopManager.h"
#include "PBShopViewModel.h"
#include "../DisplayActor/PBShopDisplayActor.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/PBShopWidget.h"
#include "../../Subsystem/BallDataSubsystem.h"
#include "Engine/GameInstance.h"
#include "../../Subsystem/AssetStreamingSubsystem.h"

// Sets default values
APBShopActor::APBShopActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void APBShopActor::OpenShop()
{
	 APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    if (!ShopManager)
    {
        ShopManager = NewObject<UPBShopManager>(this);
    	ShopPurchaseHandler = ShopManager;
    	ShopManager->SetShopActorHandler(this);
    }

    if (!ShopViewModel)
    {
        ShopViewModel = NewObject<UPBShopViewModel>(this);
    }

    if (!ShopWidget)
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

    UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
    if (!GI)
    {
        return;
    }

    UBallDataSubsystem* DataSub = GI->GetSubsystem<UBallDataSubsystem>();
    UAssetStreamingSubsystem* StreamingSub = GI->GetSubsystem<UAssetStreamingSubsystem>();

    if (!DataSub || !StreamingSub)
    {
        return;
    }

    const TArray<int32> ShopItemIds = ShopManager->OpenShop();

   

    TArray<TSoftObjectPtr<UStaticMesh>> Meshes;
    Meshes.Reserve(ShopItemIds.Num());

    for (int32 i = 0; i < ShopItemIds.Num(); ++i)
    {
        const FBallDataStruct* Data = DataSub->GetBallData(ShopItemIds[i]);
        if (!Data)
        {
            Meshes.Add(nullptr);
            continue;
        }

        const FText Name = FText::FromName(Data->NameKey);
        const FText Synergy = FText::FromName(Data->SynergyKey);

        ShopWidget->SetShopSlotWidgetData(
            i,
            Name,
            Data->BallPrice,
            Synergy
        );

        Meshes.Add(Data->BallMesh);
    }

    StreamingSub->PreLoadBallMesh(
        Meshes,
        FOnBallMeshLoaded::CreateLambda(
            [this, ShopItemIds](const TArray<UStaticMesh*>& LoadedMeshes)
            {
                if (!ShopDisplayActor)
                {
                    return;
                }

               const TArray<FVector> UIWorldLocations =  ShopDisplayActor->DisplayItems(ShopItemIds, LoadedMeshes,ShopPurchaseHandler);
            	
            	if (ShopDisplayActor && ShopWidget)
            	{
					ShopWidget->SetShopSlotWorldLocations(
						ShopDisplayActor->GetSlotWorldLocation()
					);
				}
            
            }
        )
    );

    RefreshViewModel();

    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
}

void APBShopActor::CloseShop()
{
	if (ShopWidget)
	{
		ShopWidget->RemoveFromViewport();
		ShopWidget = nullptr;
	}
	
	if (ShopDisplayActor)
	{
		ShopDisplayActor->ClearItems();
	}
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	
}

void APBShopActor::BuyItem(int32 SlotIndex)
{
	if (ShopWidget)
	{
		ShopWidget->UnActiveSlotWidget(SlotIndex);
	}
	
	RefreshViewModel();
}


void APBShopActor::RefreshViewModel()
{
	if (!ShopManager || !ShopViewModel)
	{
		return;
	}

	ShopViewModel->SetGold(ShopManager->GetCurrentGold());
}

void APBShopActor::BeginPlay()
{
	Super::BeginPlay();
	OpenShop();
}


// Called every frame


