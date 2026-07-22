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
	// 이전 호출의 데이터가 남지 않도록 초기화
	OutData = FPBPurchaseConfirmData();

	//------------------------------------------------------
	// 0. 필수 객체 확인
	//------------------------------------------------------

	if (!TableDataSubsystem)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"TableDataSubsystem is null."));

		return false;
	}

	const UGameInstance* GameInstance = GetWorld()->GetGameInstance();

	if (!GameInstance)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"GameInstance is null."));

		return false;
	}

	const UPBGameDataLoadSubsystem* LoadSubsystem =
		GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();

	if (!LoadSubsystem)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"GameDataLoadSubsystem is null."));

		return false;
	}

	//------------------------------------------------------
	// 1. SlotIndex 및 BallId 확인
	//------------------------------------------------------

	if (!CurrentShopItemBallIds.IsValidIndex(SlotIndex))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"Invalid SlotIndex=%d"),
			SlotIndex);

		return false;
	}

	const FName BallId =
		CurrentShopItemBallIds[SlotIndex];

	if (BallId.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"BallId is None. SlotIndex=%d"),
			SlotIndex);

		return false;
	}

	//------------------------------------------------------
	// 2. BallTable 조회
	//------------------------------------------------------

	FPBBallTableRow BallRow;

	if (!TableDataSubsystem->FindBallRow(
		BallId,
		BallRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"Ball row not found. BallId=%s"),
			*BallId.ToString());

		return false;
	}

	//------------------------------------------------------
	// 3. ShopTable 조회
	//------------------------------------------------------

	FPBShopTableRow ShopRow;

	if (BallRow.ShopId.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"ShopId is None. BallId=%s"),
			*BallId.ToString());

		return false;
	}

	if (!TableDataSubsystem->FindShopRow(
		BallRow.ShopId,
		ShopRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"Shop row not found. "
				"BallId=%s ShopId=%s"),
			*BallId.ToString(),
			*BallRow.ShopId.ToString());

		return false;
	}

	//------------------------------------------------------
	// 4. 기본 정보 구성
	//------------------------------------------------------

	OutData.SlotIndex = SlotIndex;
	OutData.BallName = BallRow.DisplayName;
	OutData.Description = BallRow.DescriptionKey;

	// 할인 적용 가격을 보여주려면 이쪽을 사용
	OutData.Price = CalculateDiscountedPrice(
		ShopRow.BuyPrice,
		ShopPriceDiscountAmount,
		ShopPriceDiscountPercent);

	// 할인 전 가격을 그대로 보여주려면 위 코드를 지우고:
	// OutData.Price = ShopRow.BuyPrice;

	//------------------------------------------------------
	// 5. BallDataAsset에서 볼 아이콘 조회
	//------------------------------------------------------

	const FPrimaryAssetId BallAssetId(
		PBBallAssetIds::Type::BallData,
		BallId);

	const UPBBallDataAsset* BallDataAsset =
		Cast<UPBBallDataAsset>(
			LoadSubsystem->GetLoadedPrimaryAsset(
				BallAssetId));

	if (BallDataAsset)
	{
		/*
		 * HandleShopBallAssetsLoaded()와 동일하게,
		 * 이미 UI 번들이 로드되었다는 전제라면 Get() 사용 가능.
		 *
		 * 혹시 Get()이 nullptr이면 LoadSynchronous()로 보완한다.
		 */
		OutData.BallIcon =
			BallDataAsset->BallIcon.Get();

		if (!OutData.BallIcon)
		{
			OutData.BallIcon =
				BallDataAsset->BallIcon.LoadSynchronous();
		}
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"BallDataAsset not loaded. AssetId=%s"),
			*BallAssetId.ToString());
	}

	//------------------------------------------------------
	// 6. 현재 상점 볼의 1성 스탯 조회
	//------------------------------------------------------

	FName StarLevelRowName = NAME_None;
	FPBBallStarLevelRow StarLevelRow;

	constexpr int32 DefaultStarLevel = 1;

	if (TableDataSubsystem->FindBallStarLevelRow(
		BallId,
		DefaultStarLevel,
		StarLevelRowName,
		StarLevelRow))
	{
		OutData.HP =
			StarLevelRow.BaseResources.FindRef(
				PBResourceNames::Health);

		OutData.MP =
			StarLevelRow.BaseResources.FindRef(
				PBResourceNames::Mana);

		OutData.Attack =
			StarLevelRow.BaseStats.FindRef(
				PBStatNames::Attack);

		OutData.ManaRegen =
			StarLevelRow.BaseStats.FindRef(
				PBStatNames::ManaRegen);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"BuildPurchaseConfirmData: "
				"Ball star-level row not found. "
				"BallId=%s StarLevel=%d"),
			*BallId.ToString(),
			DefaultStarLevel);
	}

	//------------------------------------------------------
	// 7. 시너지 ViewData 생성용 람다
	//------------------------------------------------------

	auto AddSynergyData =
		[this, LoadSubsystem, &OutData](
			const FName SynergyId) -> bool
		{
			if (SynergyId.IsNone())
			{
				return false;
			}

			// 동일 시너지가 이미 추가되었는지 확인
			const bool bAlreadyAdded =
				OutData.Synergies.ContainsByPredicate(
					[SynergyId](
						const FPBSynergyViewData& Data)
					{
						return Data.SynergyId == SynergyId;
					});

			if (bAlreadyAdded)
			{
				return true;
			}

			FPBSynergyTableRow SynergyRow;

			if (!TableDataSubsystem->FindSynergyRow(
				SynergyId,
				SynergyRow))
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT(
						"BuildPurchaseConfirmData: "
						"Synergy row not found. "
						"SynergyId=%s"),
					*SynergyId.ToString());

				return false;
			}

			FPBSynergyViewData SynergyViewData;

			SynergyViewData.SynergyId =
				SynergyId;

			SynergyViewData.SynergyName =
				SynergyRow.DisplayName;

			/*
			 * 현재 프로젝트에서는 DescriptionKey가 FName이므로,
			 * 기존 BallRow.DescriptionKey 처리 방식과 동일하게 넣는다.
			 *
			 * 나중에 StringTable 변환을 한곳에서 처리하게 되면
			 * 여기만 해당 함수로 바꾸면 된다.
			 */
			SynergyViewData.Description =
				FText::FromName(
					SynergyRow.DescriptionKey);

			//----------------------------------------------
			// SynergyDataAsset에서 시너지 아이콘 조회
			//----------------------------------------------

			const FPrimaryAssetId SynergyAssetId(
				PBSynergyAssetIds::Type::SynergyData,
				SynergyId);

			const UPBSynergyDataAsset* SynergyDataAsset =
				Cast<UPBSynergyDataAsset>(
					LoadSubsystem->GetLoadedPrimaryAsset(
						SynergyAssetId));

			UE_LOG(LogTemp, Warning,
	TEXT("SynergyId=%s  Asset=%s"),
	*SynergyId.ToString(),
	SynergyDataAsset ? TEXT("YES") : TEXT("NO"));
			
			if (SynergyDataAsset)
			{
				SynergyViewData.Icon =
					SynergyDataAsset->Icon.Get();

				if (!SynergyViewData.Icon)
				{
					SynergyViewData.Icon =
						SynergyDataAsset
							->Icon
							.LoadSynchronous();
				}
			}
			else
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT(
						"BuildPurchaseConfirmData: "
						"SynergyDataAsset not loaded. "
						"AssetId=%s"),
					*SynergyAssetId.ToString());
			}

			OutData.Synergies.Add(
				MoveTemp(SynergyViewData));

			return true;
		};

	//------------------------------------------------------
	// 8. 클래스 시너지 추가
	//------------------------------------------------------

	if (BallRow.ClassType != EPBBallClassType::None)
	{
		const UEnum* ClassEnum =
			StaticEnum<EPBBallClassType>();

		if (ClassEnum)
		{
			const FString ClassNameString =
				ClassEnum->GetNameStringByValue(
					static_cast<int64>(
						BallRow.ClassType));

			if (!ClassNameString.IsEmpty())
			{
				AddSynergyData(
					FName(*ClassNameString));
			}
		}
	}

	//------------------------------------------------------
	// 9. 종족 시너지 전부 추가
	//------------------------------------------------------

	const UEnum* RaceEnum =
		StaticEnum<EPBBallRaceType>();

	if (RaceEnum)
	{
		for (const EPBBallRaceType RaceType :
			BallRow.RaceTypes)
		{
			const FString RaceNameString =
				RaceEnum->GetNameStringByValue(
					static_cast<int64>(RaceType));

			if (RaceNameString.IsEmpty())
			{
				continue;
			}

			AddSynergyData(
				FName(*RaceNameString));
		}
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

