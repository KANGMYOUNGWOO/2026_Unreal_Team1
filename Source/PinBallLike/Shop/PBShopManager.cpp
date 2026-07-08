#include "PBShopManager.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

TArray<FName> UPBShopManager::OpenShop()
{
    CurrentShopItemBallIds.Empty();
    CurrentGold = 1000;

    // 임시 하드코딩.
    // 실제 RowName은 팀원 BallTable의 RowName과 반드시 일치해야 함.
    CurrentShopItemBallIds =
    {
        FName(TEXT("Ball_001")),
        FName(TEXT("Ball_002")),
        FName(TEXT("Ball_003")),
        FName(TEXT("Ball_004")),
        FName(TEXT("Ball_005")),
        FName(TEXT("Ball_006")),
        FName(TEXT("Ball_007")),
        FName(TEXT("Ball_008"))
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

    const FName SelectedBallId = CurrentShopItemBallIds[SlotIndex];

    FPBBallTableRow BallRow;
    if (!TableSub->FindBallRow(SelectedBallId, BallRow))
    {
        UE_LOG(LogTemp, Warning, TEXT("BallRow not found: %s"), *SelectedBallId.ToString());
        return false;
    }

    // TODO: 나중에 BallRow.ShopId 기반으로 ShopTable에서 가격 조회하도록 교체
    constexpr int32 TempPrice = 100;

    if (CurrentGold < TempPrice)
    {
        return false;
    }

    ShopItemIsSell[SlotIndex] = true;
    CurrentGold -= TempPrice;

    if (ShopActorHandler)
    {
        ShopActorHandler->BuyItem(SlotIndex);
    }

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