#include "PBShopManager.h"

#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Struct/Shop/PBPurChaseMessaage.h"
#include "PinBallLike/Subsystem/PBEffectSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"

#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Shop/Struct/PBShopTableRow.h"

////////////////////////////////////////////////////////////
// OpenShop

TArray<FName> UPBShopManager::OpenShop()
{
    
    RefreshActiveSynergyDiscounts();

    GenerateShopItems(8);
CurrentGold = 600;
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
    	ShopActorHandler->BuyItem(SlotIndex,false);
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
        ShopActorHandler->BuyItem(SlotIndex,true);
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

bool UPBShopManager::BuildPurchaseConfirmData(int32 SlotIndex, FPBPurchaseConfirmData& OutData) const
{
  // 이전 데이터가 남지 않도록 초기화
	OutData = FPBPurchaseConfirmData();

	if (!TableDataSubsystem)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BuildPurchaseConfirmData: TableDataSubsystem is null."));

		return false;
	}

	// 전달받은 슬롯 번호가 현재 상점 배열의 유효한 인덱스인지 확인
	if (!CurrentShopItemBallIds.IsValidIndex(SlotIndex))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BuildPurchaseConfirmData: Invalid SlotIndex: %d"),
			SlotIndex);

		return false;
	}

	// 현재 슬롯에 배치된 볼 ID
	const FName BallId = CurrentShopItemBallIds[SlotIndex];

	if (BallId.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BuildPurchaseConfirmData: BallId is None. SlotIndex: %d"),
			SlotIndex);

		return false;
	}

	// 볼 데이터 조회
	FPBBallTableRow BallRow;

	if (!TableDataSubsystem->FindBallRow(BallId, BallRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BuildPurchaseConfirmData: Ball row not found. BallId: %s"),
			*BallId.ToString());

		return false;
	}

	// 볼 데이터에 등록된 ShopId 확인
	if (BallRow.ShopId.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BuildPurchaseConfirmData: ShopId is None. BallId: %s"),
			*BallId.ToString());

		return false;
	}

	// 상점 데이터 조회
	FPBShopTableRow ShopRow;

	if (!TableDataSubsystem->FindShopRow(BallRow.ShopId, ShopRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: Shop row not found. "
				"BallId: %s, ShopId: %s"),
			*BallId.ToString(),
			*BallRow.ShopId.ToString());

		return false;
	}

	// EPBBallClassType enum 정보를 가져온다.
	const UEnum* BallClassEnum = StaticEnum<EPBBallClassType>();

	if (!BallClassEnum)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("BuildPurchaseConfirmData: StaticEnum<EPBBallClassType>() failed."));

		return false;
	}

	/*
	 * 예:
	 * EPBBallClassType::Attacker
	 *             ↓
	 * FName("Attacker")
	 */
	const FString ClassTypeString =
		BallClassEnum->GetNameStringByValue(
			static_cast<int64>(BallRow.ClassType));

	if (ClassTypeString.IsEmpty())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: Invalid ClassType value. "
				"BallId: %s"),
			*BallId.ToString());

		return false;
	}

	const FName ClassTypeName(*ClassTypeString);

	// 시너지 데이터 조회
	FPBSynergyTableRow SynergyRow;

	if (!TableDataSubsystem->FindSynergyRow(
		ClassTypeName,
		SynergyRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: Synergy row not found. "
				"BallId: %s, ClassType: %s"),
			*BallId.ToString(),
			*ClassTypeName.ToString());

		return false;
	}

	// 구매 확인 팝업에 전달할 데이터 구성
	OutData.SlotIndex = SlotIndex;
	OutData.BallId = BallId;

	OutData.BallName = BallRow.DisplayName;
	OutData.BallDescription = BallRow.DescriptionKey;

	OutData.Price = ShopRow.BuyPrice;

	OutData.SynergyName = SynergyRow.DisplayName;
	FName Desc = SynergyRow.DescriptionKey;
	FText DescText = FText::FromString(Desc.ToString());
	OutData.SynergyDescription = DescText;
	//FText Desc = 
	//OutData.SynergyDescription = Desc;

	return true;
}

void UPBShopManager::Initialize(UPBTableDataSubsystem* InTableDataSubsystem)
{
    TableDataSubsystem = InTableDataSubsystem;
	
	UGameplayMessageSubsystem& MessageSubsystem =
		  UGameplayMessageSubsystem::Get(this);
    
	
	PurChaseHandle =
		MessageSubsystem.RegisterListener<FPBPurChaseMessaage>(GameplayTags::Event_UI_Shop_Purchase,
			this,
			&UPBShopManager::HandleBuyBall); 
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

void UPBShopManager::HandleBuyBall(FGameplayTag PurChase, const FPBPurChaseMessaage& Message)
{
	BuyItem(Message.SlotIndex);
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

