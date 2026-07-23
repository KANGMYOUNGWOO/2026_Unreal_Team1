// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopActor.h"

#include <PinBallLike/Struct/Effect/PBEffectTypes.h>

#include  "../PBShopManager.h"
#include "PBShopViewModel.h"
#include "../DisplayActor/PBShopDisplayActor.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/PBShopWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "View/MVVMView.h"
#include  "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"

// Sets default values
APBShopActor::APBShopActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void APBShopActor::OpenShop()
{
	APlayerController* PC =
		UGameplayStatics::GetPlayerController(this, 0);

	if (!PC)
	{
		return;
	}

	if (!ShopManager)
	{
		ShopManager = NewObject<UPBShopManager>(this);
		ShopPurchaseHandler = ShopManager;

		ShopManager->SetShopActorHandler(this);
		
		UPBTableDataSubsystem* TableSubsystem =
			GetGameInstance()->GetSubsystem<UPBTableDataSubsystem>();
		
		UPBPlayerDataSubsystem* PlayerDataSubsystem =
			GetGameInstance()->GetSubsystem<UPBPlayerDataSubsystem>();
		
		ShopManager->Initialize(TableSubsystem,PlayerDataSubsystem);
	}

	if (!ShopWidget)
	{
		if (!ShopWidgetClass)
		{
			return;
		}

		ShopWidget =
			CreateWidget<UPBShopWidget>(
				PC,
				ShopWidgetClass);

		if (!ShopWidget)
		{
			return;
		}

		ApplyViewModelToWidget(ShopWidget);
		ShopWidget->OnRerollRequested.AddDynamic(
			this,
			&APBShopActor::HandleRerollRequested);
		ShopWidget->AddToViewport();
	}

	const TArray<FName> ShopItemBallIds =
		ShopManager->OpenShop();
	RefreshShopDisplay(ShopItemBallIds);

	RefreshViewModel();

	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	PC->SetInputMode(InputMode);
}

void APBShopActor::RefreshShopDisplay(
	const TArray<FName>& ShopItemBallIds)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBGameDataLoadSubsystem* LoadSubsystem =
		GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();

	if (!LoadSubsystem)
	{
		return;
	}

	// 비동기 로드 콜백이 뒤늦게 도착해도 최신 요청만 화면에 반영한다.
	const int32 RequestSerial = ++ShopDisplayRequestSerial;

	TArray<FPrimaryAssetId> AssetIds;
	AssetIds.Reserve(ShopItemBallIds.Num());

	for (const FName& BallId : ShopItemBallIds)
	{
		if (BallId.IsNone())
		{
			continue;
		}

		AssetIds.AddUnique(
			FPrimaryAssetId(
				PBBallAssetIds::Type::BallData,
				BallId));
	}

	// 표시할 상품이 없는 경우
	if (AssetIds.IsEmpty())
	{
		HandleShopBallAssetsLoaded(ShopItemBallIds, RequestSerial);
		return;
	}
	
	ShopAssetLoadRequestId =
		LoadSubsystem->LoadPrimaryAssetsByIdsAsync(
			AssetIds,
			{
				PBAssetBundleNames::UI
			},
			FStreamableDelegate::CreateUObject(
				this,
				&APBShopActor::HandleShopBallAssetsLoaded,
				ShopItemBallIds,
				RequestSerial));
}

void APBShopActor::HandleRerollRequested()
{
	if (!ShopManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopActor] Reroll failed. ShopManager is null."));
		return;
	}

	if (!ShopManager->RerollShop())
	{
		return;
	}

	RefreshShopDisplay(ShopManager->GetCurrentShopItemBallIds());
	RefreshViewModel();
}

void APBShopActor::CloseShop()
{
	++ShopDisplayRequestSerial;

	if (ShopWidget)
	{
		ShopWidget->RemoveFromParent();
		ShopWidget = nullptr;
	}
	
	if (ShopDisplayActor)
	{
		ShopDisplayActor->ClearItems();
	}
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	
}

void APBShopActor::BuyItem(int32 SlotIndex, bool IsEnough)
{
	if (ShopWidget)
	{
		if (IsEnough)
		{
			ShopWidget->UnActiveSlotWidget(SlotIndex);	
			ShopDisplayActor->ClearItems(SlotIndex);
		}

		else
		{
			ShopWidget->RefuseWidgetSpawn();
		}
		
	}
	
	RefreshUnsoldShopSlotWidgets();
	RefreshViewModel();
}

void APBShopActor::ShoPPurchaseConfirm(int SlotIndex)
{
	if (!ShopPurchaseHandler)
	{
		return;
	}
	
	FPBPurchaseConfirmData Data;
	ShopPurchaseHandler->BuildPurchaseConfirmData(SlotIndex,Data);
	
	
	if (ShopWidget)
	{
		ShopWidget->SetPurchaseConfirmInfo(Data);
	}
	
}

void APBShopActor::OpenAbility()
{
	OpenShop();
}

void APBShopActor::ShowNotEnoughGoldPopup()
{
}

bool APBShopActor::ApplyViewModelToWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		UE_LOG(LogTemp, Error, TEXT("Widget invalid"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Widget Class = %s"), *GetNameSafe(Widget->GetClass()));

	if (!IsValid(ShopViewModel))
	{
		ShopViewModel = NewObject<UPBShopViewModel>(this);

		if (!IsValid(ShopViewModel))
		{
			UE_LOG(LogTemp, Error, TEXT("ShopViewModel create failed"));
			return false;
		}
	}

	UMVVMView* View = Widget->GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Error, TEXT("MVVMView Extension 없음. Widget BP에 ViewModel 등록했는지 확인"));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ShopViewModel);

	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	UE_LOG(LogTemp, Warning, TEXT("SetViewModelByClass = %s"),
		bResult ? TEXT("Success") : TEXT("Failed"));

	return bResult;
}


void APBShopActor::RefreshViewModel()
{
	if (!ShopManager || !ShopViewModel)
	{
		return;
	}

	ShopViewModel->SetGold(ShopManager->GetCurrentGold());
}

void APBShopActor::RefreshUnsoldShopSlotWidgets()
{
	/*
	if (!ShopManager || !ShopWidget)
	{
		return;
	}

	UGameInstance* GameInstance =
		UGameplayStatics::GetGameInstance(GetWorld());

	if (!GameInstance)
	{
		return;
	}

	UPBTableDataSubsystem* TableSubsystem =
		GameInstance->GetSubsystem<UPBTableDataSubsystem>();

	if (!TableSubsystem)
	{
		return;
	}

	const TArray<FName>& ShopItemBallIds =
		ShopManager->GetCurrentShopItemBallIds();

	for (int32 SlotIndex = 0;
		 SlotIndex < ShopItemBallIds.Num();
		 ++SlotIndex)
	{
		if (ShopManager->IsShopItemSold(SlotIndex))
		{
			continue;
		}

		const FName BallId = ShopItemBallIds[SlotIndex];
		const int32 BuyPrice = ShopManager->GetShopItemPrice(SlotIndex);

		FPBBallTableRow BallRow;
		if (!TableSubsystem->FindBallRow(BallId, BallRow))
		{
			ShopWidget->SetShopSlotWidgetData(
				SlotIndex,
				FText::FromName(BallId),
				BuyPrice,
				FText::GetEmpty());
			continue;
		}

		ShopWidget->SetShopSlotWidgetData(
			SlotIndex,
			BallRow.DisplayName,
			BuyPrice,
			BallRow.DescriptionKey);
	}
	*/
}



void APBShopActor::HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message)
{
	if (Message.Exit == 0) CloseShop();
}

void APBShopActor::HandleShopBallAssetsLoaded(
	TArray<FName> LoadedShopItemBallIds,
	const int32 RequestSerial)
{
	if (RequestSerial != ShopDisplayRequestSerial)
	{
		UE_LOG(LogTemp, Log, TEXT("[ShopActor] Ignore stale shop asset load. RequestSerial=%d Current=%d"),
			RequestSerial,
			ShopDisplayRequestSerial);
		return;
	}

    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return;
    }

    UPBTableDataSubsystem* TableSubsystem =
        GameInstance->GetSubsystem<UPBTableDataSubsystem>();

    UPBGameDataLoadSubsystem* LoadSubsystem =
        GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();

    if (!TableSubsystem || !LoadSubsystem)
    {
        return;
    }

    TArray<UTexture2D*> BallSprites;
    BallSprites.Reserve(LoadedShopItemBallIds.Num());

    for (int32 SlotIndex = 0;
         SlotIndex < LoadedShopItemBallIds.Num();
         ++SlotIndex)
    {
        const FName BallId =
            LoadedShopItemBallIds[SlotIndex];

        UTexture2D* BallSprite = nullptr;
        UTexture2D* BallIcon = nullptr;
        int32 BuyPrice = 0;

        //--------------------------------------
        // 1. BallTable 조회
        //--------------------------------------
        FPBBallTableRow BallRow;

        if (!TableSubsystem->FindBallRow(
            BallId,
            BallRow))
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[ShopActor] Ball row not found. BallId=%s"),
                *BallId.ToString());

            // 슬롯 인덱스가 어긋나지 않도록 nullptr 추가
            BallSprites.Add(nullptr);
            continue;
        }

        //--------------------------------------
        // 2. BallRow.ShopId로 ShopTable 조회
        //--------------------------------------
        FPBShopTableRow ShopRow;

        if (BallRow.ShopId.IsNone())
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[ShopActor] ShopId is None. BallId=%s"),
                *BallId.ToString());
        }
        else if (TableSubsystem->FindShopRow(
            BallRow.ShopId,
            ShopRow))
        {
            BuyPrice = ShopRow.BuyPrice;
        }
        else
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "[ShopActor] Shop row not found. "
                    "BallId=%s ShopId=%s"),
                *BallId.ToString(),
                *BallRow.ShopId.ToString());
        }

        //--------------------------------------
        // 3. BallDataAsset 조회
        //--------------------------------------
        const FPrimaryAssetId BallAssetId(
            PBBallAssetIds::Type::BallData,
            BallId);

        const UPBBallDataAsset* BallDataAsset =
            Cast<UPBBallDataAsset>(
                LoadSubsystem->GetLoadedPrimaryAsset(
                    BallAssetId));

        if (!BallDataAsset)
        {
            const FSoftObjectPath BallDataAssetPath =
                UAssetManager::Get().GetPrimaryAssetPath(BallAssetId);
            BallDataAsset = Cast<UPBBallDataAsset>(
                BallDataAssetPath.TryLoad());
        }

        if (BallDataAsset)
        {
            BallSprite =
            	BallDataAsset->BallSprite.LoadSynchronous();

            BallIcon =
                BallDataAsset->BallIcon.LoadSynchronous();
        }
        else
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "[ShopActor] BallDataAsset not loaded. "
                    "AssetId=%s"),
                *BallAssetId.ToString());
        }

        // BallId 배열과 동일한 슬롯 순서를 유지한다.
        BallSprites.Add(BallSprite);

        //--------------------------------------
        // 4. Widget 갱신
        //--------------------------------------
        if (ShopWidget)
        {
            ShopWidget->SetShopSlotWidgetData(
                SlotIndex,
                BallRow.DisplayName,
                BuyPrice,
                BallRow.DescriptionKey,
                BallIcon);
        }
    }

    //--------------------------------------
    // 5. 월드의 ShopItemActor 생성/갱신
    //--------------------------------------
	if (ShopDisplayActor && ShopWidget)
	{
		const TArray<FVector> UIWorldLocations =
			ShopDisplayActor->DisplayItems(
				LoadedShopItemBallIds,
				BallSprites,
				ShopPurchaseHandler);

		ShopWidget->SetSlotWidgetLocation(
			UIWorldLocations);
	}
}

void APBShopActor::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayMessageSubsystem& MessageSubsystem =
	UGameplayMessageSubsystem::Get(this);
	
	ExitStartHandle =
		MessageSubsystem.RegisterListener<FPBChoiceType>(
			GameplayTags::Event_UI_Choice_Exit,
			this,
			&APBShopActor::HandleExitStart);
}



// Called every frame


