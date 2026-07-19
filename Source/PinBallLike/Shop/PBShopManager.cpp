#include "PBShopManager.h"

#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Subsystem/PBEffectSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"

#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Shop/Struct/PBShopTableRow.h"

////////////////////////////////////////////////////////////
// OpenShop

TArray<FName> UPBShopManager::OpenShop()
{
    CurrentGold = 1000;
    RefreshActiveSynergyDiscounts();

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

    return CalculateDiscountedPrice(
        ShopRow.BuyPrice,
        ShopPriceDiscountAmount,
        ShopPriceDiscountPercent);
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

    const int32 Price = CalculateDiscountedPrice(
        ShopRow.BuyPrice,
        ShopPriceDiscountAmount,
        ShopPriceDiscountPercent);

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
    RefreshActiveSynergyDiscounts();

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

////////////////////////////////////////////////////////////
// Reroll

bool UPBShopManager::RerollShop()
{
    if (CurrentGold < CurrentRerollCost)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shop] Reroll failed. Not enough gold. CurrentGold=%d Cost=%d"),
            CurrentGold,
            CurrentRerollCost);
        return false;
    }

    if (!GenerateShopItems(8))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shop] Reroll failed. Could not generate shop items."));
        return false;
    }

    CurrentGold -= CurrentRerollCost;

    if (ShopActorHandler)
    {
        //ShopActorHandler->RefreshShop();
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Shop] Reroll succeeded. CurrentGold=%d Cost=%d"),
        CurrentGold,
        CurrentRerollCost);

    return true;
}

////////////////////////////////////////////////////////////

int32 UPBShopManager::GetCurrentGold() const
{
    return CurrentGold;
}

const TArray<FName>& UPBShopManager::GetCurrentShopItemBallIds() const
{
    return CurrentShopItemBallIds;
}

bool UPBShopManager::IsShopItemSold(const int32 SlotIndex) const
{
    return ShopItemIsSold.IsValidIndex(SlotIndex) && ShopItemIsSold[SlotIndex];
}

void UPBShopManager::SetShopActorHandler(
    IIShopActorHandler* Handler)
{
    ShopActorHandler = Handler;
}

void UPBShopManager::ApplyShopPriceDiscount(const FName ModifyType, const float Value)
{
    if (FMath::IsNearlyZero(Value))
    {
        return;
    }

    if (ModifyType == PBEffectTypes::ModifyType::Add)
    {
        ShopPriceDiscountAmount = FMath::Max(0.0f, ShopPriceDiscountAmount + Value);
    }
    else
    {
        ShopPriceDiscountPercent = FMath::Clamp(
            ShopPriceDiscountPercent + Value,
            0.0f,
            100.0f);
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Shop] Price discount applied. ModifyType=%s Value=%.2f Amount=%.2f Percent=%.2f"),
        *ModifyType.ToString(),
        Value,
        ShopPriceDiscountAmount,
        ShopPriceDiscountPercent);
}

void UPBShopManager::ApplyShopRerollDiscount(const FName ModifyType, const float Value)
{
    if (FMath::IsNearlyZero(Value))
    {
        return;
    }

    if (ModifyType == PBEffectTypes::ModifyType::Add)
    {
        ShopRerollDiscountAmount = FMath::Max(0.0f, ShopRerollDiscountAmount + Value);
    }
    else
    {
        ShopRerollDiscountPercent = FMath::Clamp(
            ShopRerollDiscountPercent + Value,
            0.0f,
            100.0f);
    }

    CurrentRerollCost = CalculateDiscountedPrice(
        BaseRerollCost,
        ShopRerollDiscountAmount,
        ShopRerollDiscountPercent);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Shop] Reroll discount applied. ModifyType=%s Value=%.2f Amount=%.2f Percent=%.2f Cost=%d"),
        *ModifyType.ToString(),
        Value,
        ShopRerollDiscountAmount,
        ShopRerollDiscountPercent,
        CurrentRerollCost);
}

void UPBShopManager::RefreshActiveSynergyDiscounts()
{
    ShopPriceDiscountPercent = 0.0f;
    ShopPriceDiscountAmount = 0.0f;
    ShopRerollDiscountPercent = 0.0f;
    ShopRerollDiscountAmount = 0.0f;
    CurrentRerollCost = BaseRerollCost;
    
    UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
    if (!GI)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Failed to apply synergy effects. GameInstance is null."));
        return;
    }

    UPBEffectSubsystem* EffectSubsystem = GI->GetSubsystem<UPBEffectSubsystem>();
    if (!EffectSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Failed to apply synergy effects. EffectSubsystem is null."));
        return;
    }

    FPBEffectContext EffectContext;
    EffectContext.WorldContextObject = this;
    const int32 AppliedCount = EffectSubsystem->NotifyTrigger(
        GameplayTags::TriggerEvent_Shop_Opened,
        EffectContext);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Shop] Active synergy shop effects applied. Count=%d PriceDiscountAmount=%.2f PriceDiscountPercent=%.2f RerollDiscountAmount=%.2f RerollDiscountPercent=%.2f RerollCost=%d"),
        AppliedCount,
        ShopPriceDiscountAmount,
        ShopPriceDiscountPercent,
        ShopRerollDiscountAmount,
        ShopRerollDiscountPercent,
        CurrentRerollCost);
}

int32 UPBShopManager::CalculateDiscountedPrice(
    const int32 BasePrice,
    const float DiscountAmount,
    const float DiscountPercent) const
{
    if (BasePrice <= 0)
    {
        return 0;
    }

    const float PriceAfterAmountDiscount = FMath::Max(
        0.0f,
        static_cast<float>(BasePrice) - FMath::Max(0.0f, DiscountAmount));
    const float ClampedDiscountPercent = FMath::Clamp(DiscountPercent, 0.0f, 100.0f);
    return FMath::Max(
        0,
        FMath::RoundToInt(PriceAfterAmountDiscount * (1.0f - ClampedDiscountPercent * 0.01f)));
}
