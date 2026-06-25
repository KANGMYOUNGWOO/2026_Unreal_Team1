// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopDisplayActor.h"

#include "PBShopItemActor.h"

// Sets default values
APBShopDisplayActor::APBShopDisplayActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	
	constexpr int32 Slotcount = 8;
	
	for(int32 i = 0; i < Slotcount; ++i)
	{
		const FString SlotName = FString::Printf(TEXT("ShopSlot_%d"), i);
		
		USceneComponent* Slot =
			CreateDefaultSubobject<USceneComponent>(*SlotName);
		
		Slot->SetupAttachment(Root);
		
		const int32 Row = i / 4;
		const int32 Col = i % 4;
		
		Slot->SetRelativeLocation(FVector(0.f, Col * 200.0f, Row * 200.0f));
		
		ShopSlots.Add(Slot);
	}

}

void APBShopDisplayActor::DisplayItems(TArray<int32> ItemsID)
{
	
	for(APBShopItemActor* ItemActor : SpawnedItems)
	{
		if(ItemActor)
		{
			ItemActor->Destroy();
		}
	}
	
	SpawnedItems.Empty();
	
	if (!ShopItemActorClass)
	{
		return;
	}
	
	const int32 Count = FMath::Min(ItemsID.Num(),ShopSlots.Num());

	for (int32 i =0; i < Count; ++i)
	{
		if (!ShopSlots[i])
		{
			continue;
		}
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		
		APBShopItemActor* SpawnedItem = GetWorld()->SpawnActor<APBShopItemActor>(
			ShopItemActorClass,ShopSlots[i]->GetComponentTransform(),
			SpawnParams);
		
		if (SpawnedItem)
		{
			//SpawnedItem->SetBallData(Items[i]);
			SpawnedItems.Add(SpawnedItem);
		}
			
	}
	
}

TArray<FVector> APBShopDisplayActor::GetSlotWorldLocation() const
{
	TArray<FVector> Result;
	for (int i=0 ; i<ShopSlots.Num(); ++i)
	{
		Result.Add(ShopSlots[i]->GetComponentLocation());
	}
	
	return Result;
}

void APBShopDisplayActor::ClearItems()
{
}

// Called when the game starts or when spawned
void APBShopDisplayActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APBShopDisplayActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

