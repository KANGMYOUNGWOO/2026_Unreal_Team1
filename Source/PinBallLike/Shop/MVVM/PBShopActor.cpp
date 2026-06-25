// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopActor.h"

#include  "../PBShopManager.h"
#include "PBShopViewModel.h"
#include "../DisplayActor/PBShopDisplayActor.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/PBShopWidget.h"

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
	}
	
	if (!ShopViewModel)
	{
		ShopViewModel = NewObject<UPBShopViewModel>(this);
	}
	
	if (!ShopWidget)
	{
		if (!ShopWidgetClass)
		{
			
		}
	}
	
}

void APBShopActor::ClostShop()
{
}

void APBShopActor::RefreshShopView()
{
}

// Called when the game starts or when spawned
void APBShopActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APBShopActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

