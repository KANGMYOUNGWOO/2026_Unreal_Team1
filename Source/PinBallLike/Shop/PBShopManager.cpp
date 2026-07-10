#include "PBShopManager.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"

#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

TArray<FName> UPBShopManager::OpenShop()
{
    CurrentShopItemBallIds.Empty();
    CurrentGold = 1000;

    // 임시 상품 목록.
    // 반드시 BallTable에 실제 존재하는 RowName이어야 한다.
    CurrentShopItemBallIds =
    {
        FName(TEXT("Ball_Test01")),
        FName(TEXT("Ball_Test02")),
        FName(TEXT("Ball_Test01")),
        FName(TEXT("Ball_Test02")),
        FName(TEXT("Ball_Test01")),
        FName(TEXT("Ball_Test02")),
        FName(TEXT("Ball_Test01")),
        FName(TEXT("Ball_Test02"))
    };

    ShopItemIsSell.Init(false, CurrentShopItemBallIds.Num());

    return CurrentShopItemBallIds;
}

bool UPBShopManager::BuyItem(int32 SlotIndex)
{
    if (!CurrentShopItemBallIds.IsValidIndex(SlotIndex))
    {
        return false;
    }

    if (ShopItemIsSell.IsValidIndex(SlotIndex) && ShopItemIsSell[SlotIndex])
    {
        return false;
    }

    UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
    if (!GI)
    {
        return false;
    }

    UPBTableDataSubsystem* TableSub = GI->GetSubsystem<UPBTableDataSubsystem>();
    if (!TableSub)
    {
        return false;
    }

    UPBBallDeckSubsystem* DeckSubsystem = GI->GetSubsystem<UPBBallDeckSubsystem>();
    if (!DeckSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuyItem failed: DeckSubsystem is null."));
        return false;
    }

    const FName SelectedBallId = CurrentShopItemBallIds[SlotIndex];

    FPBBallTableRow BallRow;
    if (!TableSub->FindBallRow(SelectedBallId, BallRow))
    {
        UE_LOG(LogTemp, Warning, TEXT("BallRow not found: %s"), *SelectedBallId.ToString());
        return false;
    }

    constexpr int32 TempPrice = 100;

    if (CurrentGold < TempPrice)
    {
        UE_LOG(LogTemp, Warning, TEXT("돈 부족 : Gold=%d Price=%d"), CurrentGold, TempPrice);
        return false;
    }

    if (!DeckSubsystem->AddNewBallToDeck(SelectedBallId))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("BuyItem failed: AddNewBallToDeck failed. BallId=%s"),
            *SelectedBallId.ToString());
        return false;
    }

    DeckSubsystem->LoadPlacedBallUIAssetsAsync(FStreamableDelegate());
    DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate());

    ShopItemIsSell[SlotIndex] = true;
    CurrentGold -= TempPrice;

    if (ShopActorHandler)
    {
        ShopActorHandler->BuyItem(SlotIndex);
    }

    UE_LOG(LogTemp, Warning,
        TEXT("BuyItem succeeded. Slot=%d BallId=%s Gold=%d"),
        SlotIndex,
        *SelectedBallId.ToString(),
        CurrentGold);

    return true;
}

int32 UPBShopManager::GetCurrentGold() const
{
    return CurrentGold;
}

void UPBShopManager::SetShopActorHandler(IIShopActorHandler* Handler)
{
    ShopActorHandler = Handler;
}