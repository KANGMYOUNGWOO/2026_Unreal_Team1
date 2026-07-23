#include "PBShopManager.h"

#include "Engine/AssetManager.h"
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
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"

#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/Struct/PBBallStarLevelRow.h"

#include "PinBallLike/Table/Synergy/PBSynergyAssetIds.h"
#include "PinBallLike/Table/Synergy/DataAsset/PBSynergyDataAsset.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"

#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
////////////////////////////////////////////////////////////
// OpenShop

TArray<FName> UPBShopManager::OpenShop()
{
    
	UGameInstance* GI =
	 UGameplayStatics::GetGameInstance(GetWorld());

	if (GI)
	{
		UPBGameDataLoadSubsystem* LoadSubsystem =
			GI->GetSubsystem<UPBGameDataLoadSubsystem>();

		if (LoadSubsystem)
		{
			LoadSubsystem->LoadPrimaryAssetTypeAsync(
				PBSynergyAssetIds::Type::SynergyData,
				{ FName(TEXT("UI")) });
		}
	}

	RefreshActiveSynergyDiscounts();
	GenerateShopItems(8);

	
	return CurrentShopItemBallIds;
}

////////////////////////////////////////////////////////////
// GenerateShopItems

FPBBallDetailInfoRowViewData UPBShopManager::MakeInfoRow(const FText& LabelText, int32 Value) const
{
	FPBBallDetailInfoRowViewData RowViewData;
	RowViewData.LabelText = LabelText;
	RowViewData.ValueText = FText::AsNumber(Value);
	return RowViewData;
}

FPBBallDetailIconTextViewData UPBShopManager::MakeIconText(UTexture2D* IconTexture, const FText& Text) const
{
	FPBBallDetailIconTextViewData outViewData;
	outViewData.IconTexture = IconTexture;
	outViewData.Text = Text;
	return outViewData;
}

int32 UPBShopManager::FindMapValue(const TMap<FName, int32>& Values, FName Key) const
{
	const int32* Value = Values.Find(Key);
	return Value ? *Value : 0;
}

FText UPBShopManager::GetEnumDisplayText(const UEnum* Enum, int64 Value) const
{
	return Enum ? Enum->GetDisplayNameTextByValue(Value) : FText::GetEmpty();
}

FName UPBShopManager::GetEnumValueName(const UEnum* Enum, int64 Value) const
{
	return Enum ? FName(*Enum->GetNameStringByValue(Value)) : NAME_None;
}

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

    if (PlayerDataSubsystem->GetCurrentGold() < Price)
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

    PlayerDataSubsystem->SpendGold(Price);
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

    if (PlayerDataSubsystem->GetCurrentGold()< Price)
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
    if (PlayerDataSubsystem->GetCurrentGold() < CurrentRerollCost)
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

	PlayerDataSubsystem->SpendGold(CurrentRerollCost);

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

bool UPBShopManager::BuildPurchaseConfirmData(
	int32 SlotIndex,
	FPBPurchaseConfirmData& OutData) const
{
	OutData = FPBPurchaseConfirmData();

	//------------------------------------------------------
	// 0. Subsystem 확인
	//------------------------------------------------------

	if (!TableDataSubsystem)
	{
		return false;
	}

	const UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	//------------------------------------------------------
	// 1. SlotIndex 검사
	//------------------------------------------------------

	if (!CurrentShopItemBallIds.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const FName BallId = CurrentShopItemBallIds[SlotIndex];

	if (BallId.IsNone())
	{
		return false;
	}

	//------------------------------------------------------
	// 2. BallTable
	//------------------------------------------------------

	FPBBallTableRow BallRow;
	if (!TableDataSubsystem->FindBallRow(BallId, BallRow))
	{
		return false;
	}

	//------------------------------------------------------
	// 3. ShopTable
	//------------------------------------------------------

	FPBShopTableRow ShopRow;
	if (!TableDataSubsystem->FindShopRow(BallRow.ShopId, ShopRow))
	{
		return false;
	}

	//------------------------------------------------------
	// 4. StarLevel
	//------------------------------------------------------

	FName StarLevelRowName = NAME_None;
	FPBBallStarLevelRow StarLevelRow;

	constexpr int32 DefaultStarLevel = 1;

	if (!TableDataSubsystem->FindBallStarLevelRow(
		BallId,
		DefaultStarLevel,
		StarLevelRowName,
		StarLevelRow))
	{
		return false;
	}

	//------------------------------------------------------
	// 5. Skill
	//------------------------------------------------------

	FPBBallSkillTableRow SkillRow;
	TableDataSubsystem->FindDefaultSkillRowForBall(
		BallId,
		SkillRow);

	//------------------------------------------------------
	// 6. Enum
	//------------------------------------------------------

	const UEnum* PowerFlipEnum = StaticEnum<EPBPowerFlipType>();
	const UEnum* RaceEnum = StaticEnum<EPBBallRaceType>();
	const UEnum* ClassEnum = StaticEnum<EPBBallClassType>();

	//------------------------------------------------------
	// 7. BallDataAsset
	//------------------------------------------------------

	const FPrimaryAssetId BallAssetId(
		PBBallAssetIds::Type::BallData,
		BallId);

	FAssetData BallAssetData;
	const UPBBallDataAsset* BallDataAsset = nullptr;

	if (UAssetManager::Get().GetPrimaryAssetData(
		BallAssetId,
		BallAssetData))
	{
		BallDataAsset =
			Cast<UPBBallDataAsset>(
				BallAssetData.GetAsset());
	}

	//------------------------------------------------------
	// 8. 기본 정보
	//------------------------------------------------------

	OutData.SlotIndex = SlotIndex;

	OutData.Price = CalculateDiscountedPrice(
		ShopRow.BuyPrice,
		ShopPriceDiscountAmount,
		ShopPriceDiscountPercent);

	OutData.bHasBall = true;
	OutData.BallId = BallId;

	OutData.BallIconTexture = nullptr;

	if (BallDataAsset)
	{
		OutData.BallIconTexture =
			BallDataAsset->BallIcon.LoadSynchronous();
	}

	OutData.BallNameText =
		BallRow.DisplayName.IsEmpty()
			? FText::FromName(BallId)
			: BallRow.DisplayName;

	OutData.BallDescriptionText =
		BallRow.DescriptionKey;

	//------------------------------------------------------
	// 9. 능력치
	//------------------------------------------------------

	OutData.HpRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "HPLabel", "HP"),
		FindMapValue(
			StarLevelRow.BaseResources,
			PBResourceNames::Health));

	OutData.MpRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "MPLabel", "MP"),
		FindMapValue(
			StarLevelRow.BaseResources,
			PBResourceNames::Mana));

	OutData.AttackRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "AttackLabel", "공격력"),
		FindMapValue(
			StarLevelRow.BaseStats,
			PBStatNames::Attack));

	OutData.ManaRegenRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "ManaRegenLabel", "MP회복"),
		FindMapValue(
			StarLevelRow.BaseStats,
			PBStatNames::ManaRegen));

	//------------------------------------------------------
	// 10. PowerFlip
	//------------------------------------------------------

	OutData.PowerFlipData = MakeIconText(
		BallDataAsset
			? BallDataAsset->PowerFlipIcon.LoadSynchronous()
			: nullptr,
		GetEnumDisplayText(
			PowerFlipEnum,
			static_cast<int64>(BallRow.PowerFlipType)));

	//------------------------------------------------------
	// 11. Class
	//------------------------------------------------------

	if (BallRow.ClassType != EPBBallClassType::None)
	{
		OutData.ClassData = MakeIconText(
			BallDataAsset
				? BallDataAsset->ClassIcon.LoadSynchronous()
				: nullptr,
			GetEnumDisplayText(
				ClassEnum,
				static_cast<int64>(BallRow.ClassType)));
	}

	//------------------------------------------------------
	// 12. Race
	//------------------------------------------------------

	for (int32 RaceIndex = 0;
		 RaceIndex < BallRow.RaceTypes.Num();
		 ++RaceIndex)
	{
		const EPBBallRaceType RaceType =
			BallRow.RaceTypes[RaceIndex];

		OutData.RaceDataList.Add(
			MakeIconText(
				(BallDataAsset &&
				 BallDataAsset->RaceIcons.IsValidIndex(RaceIndex))
					? BallDataAsset->RaceIcons[RaceIndex].LoadSynchronous()
					: nullptr,
				GetEnumDisplayText(
					RaceEnum,
					static_cast<int64>(RaceType))));
	}

	//------------------------------------------------------
	// 13. Skill
	//------------------------------------------------------

	OutData.SkillNameText =
		SkillRow.DisplayName;

	OutData.SkillDescriptionText =
		SkillRow.GetDescription(
			FindMapValue(
				StarLevelRow.BaseStats,
				PBStatNames::Attack));

	if (BallDataAsset)
	{
		OutData.SkillIconTexture =
			BallDataAsset->SkillIcon.LoadSynchronous();
	}

	return true;
}

void UPBShopManager::Initialize(UPBTableDataSubsystem* InTableDataSubsystem, UPBPlayerDataSubsystem* InPlayerDataSubSystem)
{
    TableDataSubsystem = InTableDataSubsystem;
	PlayerDataSubsystem = InPlayerDataSubSystem;
	
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
	 return PlayerDataSubsystem->GetCurrentGold();
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

