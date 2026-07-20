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

void APBShopActor::RefreshShopDisplay(const TArray<FName>& ShopItemBallIds)
{
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

	UStaticMesh* CubeMesh =
		LoadObject<UStaticMesh>(
			nullptr,
			TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (!CubeMesh)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[ShopActor] Default cube mesh load failed."));

		return;
	}

	TArray<UStaticMesh*> LoadedMeshes;
	LoadedMeshes.Reserve(ShopItemBallIds.Num());

	for (int32 SlotIndex = 0;
		 SlotIndex < ShopItemBallIds.Num();
		 ++SlotIndex)
	{
		const FName BallId =
			ShopItemBallIds[SlotIndex];

		const int32 BuyPrice =
			ShopManager->GetShopItemPrice(SlotIndex);

		FPBBallTableRow BallRow;
		if (!TableSubsystem->FindBallRow(
			BallId,
			BallRow))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"[ShopActor] Ball row not found. "
					"Slot=%d BallId=%s"),
				SlotIndex,
				*BallId.ToString());

			LoadedMeshes.Add(CubeMesh);

			ShopWidget->SetShopSlotWidgetData(
				SlotIndex,
				FText::FromName(BallId),
				BuyPrice,
				FText::GetEmpty());

			continue;
		}

		LoadedMeshes.Add(CubeMesh);

		ShopWidget->SetShopSlotWidgetData(
			SlotIndex,
			BallRow.DisplayName,
			BuyPrice,
			BallRow.DescriptionKey);
	}

	if (ShopDisplayActor)
	{
		const TArray<FVector> UIWorldLocations =
			ShopDisplayActor->DisplayItems(
				ShopItemBallIds,
				LoadedMeshes,
				ShopPurchaseHandler);

		ShopWidget->SetShopSlotWorldLocations(
			UIWorldLocations);
	}
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

void APBShopActor::BuyItem(int32 SlotIndex)
{
	if (ShopWidget)
	{
		ShopWidget->UnActiveSlotWidget(SlotIndex);
	}
	
	RefreshUnsoldShopSlotWidgets();
	RefreshViewModel();
}

void APBShopActor::ShoPPurchaseConfirm(int SlotIndex)
{
	if (ShopWidget)
	{
		ShopWidget->SetPurchaseConfirmInfo();
	}
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

void APBShopActor::RefreshUnsoldShopSlotWidgets()
{
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


