// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopActor.h"
#include  "../PBShopManager.h"
#include "PBShopViewModel.h"
#include "../DisplayActor/PBShopDisplayActor.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/PBShopWidget.h"

#include "Engine/GameInstance.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "View/MVVMView.h"
#include  "PinBallLike/GamePlayTag/GamePlayTags.h"

// Sets default values
APBShopActor::APBShopActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void APBShopActor::OpenShop()
{
	 APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    if (!ShopManager)
    {
        ShopManager = NewObject<UPBShopManager>(this);
    	ShopPurchaseHandler = ShopManager;
    	ShopManager->SetShopActorHandler(this);
    }
	
    if (!ShopWidget)
    {
        if (!ShopWidgetClass)
        {
            return;
        }

        ShopWidget = CreateWidget<UPBShopWidget>(PC, ShopWidgetClass);
        if (!ShopWidget)
        {
            return;
        }
    	
    	ApplyViewModelToWidget(ShopWidget);
    	
    	ShopWidget->AddToViewport();
    }

    UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
    if (!GI)
    {
        return;
    }

	UPBTableDataSubsystem* TableSub = GI->GetSubsystem<UPBTableDataSubsystem>();

	if (!TableSub)
	{
		return;
	}

	const TArray<FName> ShopItemIds = ShopManager->OpenShop();

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(
		nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);

	if (!CubeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Default Cube Mesh load failed"));
		return;
	}

	TArray<UStaticMesh*> LoadedMeshes;
	LoadedMeshes.Reserve(ShopItemIds.Num());

	for (int32 i = 0; i < ShopItemIds.Num(); ++i)
	{
		FPBBallTableRow BallRow;

		if (!TableSub->FindBallRow(ShopItemIds[i], BallRow))
		{
			LoadedMeshes.Add(CubeMesh);

			ShopWidget->SetShopSlotWidgetData(
				i,
				FText::FromName(ShopItemIds[i]),
				100,
				FText::GetEmpty()
			);

			continue;
		}

		const FText Name = BallRow.DisplayName;

		const FText Synergy =
			BallRow.SynergyIds.Num() > 0
				? FText::FromName(BallRow.SynergyIds[0])
				: FText::GetEmpty();

		// TODO: 가격은 나중에 ShopId 기반 ShopTable에서 가져오도록 교체
		const int32 TempPrice = 100;

		ShopWidget->SetShopSlotWidgetData(
			i,
			Name,
			TempPrice,
			Synergy
		);

		LoadedMeshes.Add(CubeMesh);
	}

	if (ShopDisplayActor)
	{
		ShopDisplayActor->DisplayItems(
			ShopItemIds,
			LoadedMeshes,
			ShopPurchaseHandler
		);

		if (ShopWidget)
		{
			ShopWidget->SetShopSlotWorldLocations(
				ShopDisplayActor->GetSlotWorldLocation()
			);
		}
	}

    RefreshViewModel();

    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
}

void APBShopActor::CloseShop()
{
	if (ShopWidget)
	{
		ShopWidget->RemoveFromViewport();
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

void APBShopActor::BuyItem(int32 SlotIndex)
{
	if (ShopWidget)
	{
		ShopWidget->UnActiveSlotWidget(SlotIndex);
	}
	
	RefreshViewModel();
}

void APBShopActor::OpenAbility()
{
	OpenShop();
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

void APBShopActor::HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message)
{
	if (Message.Exit == 0) CloseShop();
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


