#include "PBShopManager.h"

#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"

#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Shop/Struct/PBShopTableRow.h"

////////////////////////////////////////////////////////////
// OpenShop

TArray<FName> UPBShopManager::OpenShop()
{
   

    GenerateShopItems(8);

    return CurrentShopItemBallIds;
}

////////////////////////////////////////////////////////////
// GenerateShopItems

bool UPBShopManager::GenerateShopItems(const int32 SlotCount)
{
    UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
    if (!GI)
    {
        return false;
    }

    UPBTableDataSubsystem* TableSub =
        GI->GetSubsystem<UPBTableDataSubsystem>();

    if (!TableSub)
    {
        return false;
    }

    //------------------------------------------------------
    // 기존 데이터를 바로 지우지 않는다.
    //------------------------------------------------------

    TArray<FName> NewBallIds;
    TArray<FName> NewShopRows;

    //------------------------------------------------------
    // ShopTable의 모든 Row 가져오기
    //------------------------------------------------------

    TArray<FName> AllShopRows;
    TableSub->GetAllShopRowName(AllShopRows);

    if (AllShopRows.IsEmpty())
    {
        return false;
    }

    //------------------------------------------------------
    // 랜덤 생성
    //------------------------------------------------------

    for (int32 i = 0; i < SlotCount; i++)
    {
        const int32 RandomIndex =
            FMath::RandRange(
                0,
                AllShopRows.Num() - 1);

        const FName SelectedRow =
            AllShopRows[RandomIndex];

        FPBShopTableRow ShopRow;

        if (!TableSub->FindShopRow(
            SelectedRow,
            ShopRow))
        {
            continue;
        }

        NewShopRows.Add(SelectedRow);
        NewBallIds.Add(ShopRow.BallKey);
    }

    //------------------------------------------------------
    // 생성 실패
    //------------------------------------------------------

    if (NewBallIds.Num() != SlotCount)
    {
        return false;
    }

    //------------------------------------------------------
    // 성공 시 교체
    //------------------------------------------------------

    CurrentShopItemBallIds = MoveTemp(NewBallIds);
    CurrentShopItemRowNames = MoveTemp(NewShopRows);

    ShopItemIsSold.Init(
        false,
        CurrentShopItemBallIds.Num());

    return true;
}

int32 UPBShopManager::GetShopItemPrice(int32 SlotIndex) const
{
    if (!CurrentShopItemRowNames.IsValidIndex(SlotIndex))
    {
        return 0;
    }

    const UGameInstance* GameInstance =
        UGameplayStatics::GetGameInstance(GetWorld());

    if (!GameInstance)
    {
        return 0;
    }

    const UPBTableDataSubsystem* TableSubsystem =
        GameInstance->GetSubsystem<UPBTableDataSubsystem>();

    if (!TableSubsystem)
    {
        return 0;
    }

    FPBShopTableRow ShopRow;
    if (!TableSubsystem->FindShopRow(
        CurrentShopItemRowNames[SlotIndex],
        ShopRow))
    {
        return 0;
    }

    return ShopRow.BuyPrice;
}

FName UPBShopManager::GetShopItemRowName(int32 SlotIndex) const
{
    return CurrentShopItemRowNames.IsValidIndex(SlotIndex)
        ? CurrentShopItemRowNames[SlotIndex]
        : NAME_None;
}

////////////////////////////////////////////////////////////
// BuyItem

bool UPBShopManager::BuyItem(int32 SlotIndex)
{
    if (!CurrentShopItemBallIds.IsValidIndex(SlotIndex))
    {
        return false;
    }

    if (ShopItemIsSold[SlotIndex])
    {
        return false;
    }

    UGameInstance* GI =
        UGameplayStatics::GetGameInstance(GetWorld());

    if (!GI)
    {
        return false;
    }

    UPBTableDataSubsystem* TableSub =
        GI->GetSubsystem<UPBTableDataSubsystem>();

    UPBBallDeckSubsystem* DeckSubsystem =
        GI->GetSubsystem<UPBBallDeckSubsystem>();

    if (!TableSub || !DeckSubsystem)
    {
        return false;
    }

    //------------------------------------------------------
    // ShopRow 가져오기
    //------------------------------------------------------

    FPBShopTableRow ShopRow;

    if (!TableSub->FindShopRow(
        CurrentShopItemRowNames[SlotIndex],
        ShopRow))
    {
        return false;
    }

    const int32 Price = ShopRow.BuyPrice;

    //------------------------------------------------------
    // 골드 검사
    //------------------------------------------------------

    if (CurrentGold < Price)
    {
        return false;
    }

    //------------------------------------------------------
    // Ball 획득
    //------------------------------------------------------

    if (!DeckSubsystem->AddNewBallToDeck(
        ShopRow.BallKey))
    {
        return false;
    }

    DeckSubsystem->LoadPlacedBallUIAssetsAsync(
        FStreamableDelegate());

    DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(
        FStreamableDelegate());

    //------------------------------------------------------

    ShopItemIsSold[SlotIndex] = true;

    CurrentGold -= Price;

    if (ShopActorHandler)
    {
        ShopActorHandler->BuyItem(SlotIndex);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BuyItem : %s"),
        *ShopRow.BallKey.ToString());

    return true;
}

void UPBShopManager::RequestPurchase(int32 SlotIndex)
{
 
    if (!CurrentShopItemBallIds.IsValidIndex(SlotIndex))
    {
        return;
    }

    if (ShopItemIsSold[SlotIndex])
    {
        return;
    }

    UGameInstance* GI =
        UGameplayStatics::GetGameInstance(GetWorld());

    if (!GI)
    {
        return;
    }

    UPBTableDataSubsystem* TableSub =
        GI->GetSubsystem<UPBTableDataSubsystem>();

    UPBBallDeckSubsystem* DeckSubsystem =
        GI->GetSubsystem<UPBBallDeckSubsystem>();

    if (!TableSub || !DeckSubsystem)
    {
        return;
    }

    //------------------------------------------------------
    // ShopRow 가져오기
    //------------------------------------------------------

    FPBShopTableRow ShopRow;

    if (!TableSub->FindShopRow(
        CurrentShopItemRowNames[SlotIndex],
        ShopRow))
    {
        return;
    }

    const int32 Price = ShopRow.BuyPrice;

    //------------------------------------------------------
    // 골드 검사
    //------------------------------------------------------

    if (CurrentGold < Price)
    {
       ShopActorHandler->ShowNotEnoughGoldPopup();
        return;
    }
    
    if (ShopActorHandler)
    {
        ShopActorHandler-> ShoPPurchaseConfirm(SlotIndex);
    }
}


////////////////////////////////////////////////////////////
// Reroll

bool UPBShopManager::RerollShop()
{
    constexpr int32 RerollCost = 50;

    if (CurrentGold < RerollCost)
    {
        return false;
    }

    if (!GenerateShopItems(8))
    {
        return false;
    }

    CurrentGold -= RerollCost;

    if (ShopActorHandler)
    {
        //ShopActorHandler->RefreshShop();
    }

    return true;
}

////////////////////////////////////////////////////////////

int32 UPBShopManager::GetCurrentGold() const
{
    return CurrentGold;
}

void UPBShopManager::SetShopActorHandler(
    IIShopActorHandler* Handler)
{
    ShopActorHandler = Handler;
}