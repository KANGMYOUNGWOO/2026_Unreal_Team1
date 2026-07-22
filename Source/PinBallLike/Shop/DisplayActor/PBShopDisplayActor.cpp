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

TArray<FVector> APBShopDisplayActor::DisplayItems(
    const TArray<FName>& ItemIds,
    const TArray<UTexture2D*>& Sprites,
    IIShopPurchaseHandler* Handler)
{
    TArray<FVector> UIWorldLocations;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShopDisplay] ItemIds=%d, Sprites=%d, Slots=%d"),
        ItemIds.Num(),
        Sprites.Num(),
        ShopSlots.Num());

    // 기존 아이템 제거
    for (APBShopItemActor* ItemActor : SpawnedItems)
    {
        if (IsValid(ItemActor))
        {
            ItemActor->Destroy();
        }
    }

    const int32 Count = FMath::Min3(
        ItemIds.Num(),
        Sprites.Num(),
        ShopSlots.Num());

    // ★ 인덱스를 미리 확보
    SpawnedItems.Empty();
    SpawnedItems.SetNum(Count);

    UIWorldLocations.Empty();
    UIWorldLocations.SetNum(Count);

    if (!ShopItemActorClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShopDisplay] ShopItemActorClass is null"));

        return UIWorldLocations;
    }

    for (int32 Index = 0; Index < Count; ++Index)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShopDisplay] Index=%d BallId=%s Sprite=%s"),
            Index,
            *ItemIds[Index].ToString(),
            *GetNameSafe(Sprites[Index]));

        if (!ShopSlots[Index])
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("[ShopDisplay] Slot is null. Index=%d"),
                Index);

            SpawnedItems[Index] = nullptr;
            continue;
        }

        // Sprite가 없으면 Spawn하지 않되 인덱스는 유지
        if (!Sprites[Index])
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[ShopDisplay] Sprite is null. Index=%d"),
                Index);

            SpawnedItems[Index] = nullptr;
            UIWorldLocations[Index] =
                ShopSlots[Index]->GetComponentLocation()
        	+ FVector(0.f, 0.f, 30.f);

            continue;
        }

        const FTransform SpawnTransform =
            ShopSlots[Index]->GetComponentTransform();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APBShopItemActor* SpawnedItem =
            GetWorld()->SpawnActor<APBShopItemActor>(
                ShopItemActorClass,
                SpawnTransform,
                SpawnParams);

        if (!SpawnedItem)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("[ShopDisplay] Spawn failed. Index=%d"),
                Index);

            SpawnedItems[Index] = nullptr;
            UIWorldLocations[Index] =
                ShopSlots[Index]->GetComponentLocation()
        	+ FVector(0.f, 0.f, 30.f);;
        	 

            continue;
        }

        SpawnedItem->SetSlotIndex(Index);
        SpawnedItem->SetSprite(Sprites[Index]);
        SpawnedItem->SetHandler(Handler);

        // ★ Add()가 아니라 대입
        SpawnedItems[Index] = SpawnedItem;
    	UIWorldLocations[Index] =
		 ShopSlots[Index]->GetComponentLocation()
			 + FVector(0.f, 0.f, 30.f);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Spawn Success Index=%d"),
            Index);
    }

    return UIWorldLocations;
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
	for (APBShopItemActor* ItemActor : SpawnedItems)
	{
		if (IsValid(ItemActor))
		{
			ItemActor->Destroy();
		}
	}

	SpawnedItems.Empty();
}

void APBShopDisplayActor::ClearItems(int32 SlotIndex)
{
	SpawnedItems[SlotIndex]->OnPurchase();
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

