// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopWidget.h"

#include "PBPurchaseConfirmWidget.h"
#include "PBShopRefuseWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components//Button.h"
#include "Components/CanvasPanelSlot.h"
#include "PBShopSlotWidget.h"
#include "PinBallLike/Deck/UI/View/PBDeckOverviewWidget.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "PinBallLike/Struct/Choice/PBChoiceType.h"

void UPBShopWidget::UpdateSlotWidgetPositionsOnce()
{
    APlayerController* PC = GetOwningPlayer();

    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopUI] OwningPlayer is null"));
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShopUI] Locations=%d Widgets=%d Offset=%s"),
        ShopSlotWorldLocations.Num(),
        ShopSlotWidgets.Num(),
        *SlotWidgetWorldOffset.ToString());

    int32 ViewportSizeX = 0;
    int32 ViewportSizeY = 0;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShopUI] Viewport=%d x %d"),
        ViewportSizeX,
        ViewportSizeY);

    const int32 Count =
        FMath::Min(ShopSlotWorldLocations.Num(), ShopSlotWidgets.Num());

    for (int32 Index = 0; Index < Count; ++Index)
    {
        UPBShopSlotWidget* SlotWidget = ShopSlotWidgets[Index];

        if (!SlotWidget)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("[ShopUI] Widget null. Index=%d"),
                Index);
            continue;
        }

        const FVector SourceWorldLocation =
            ShopSlotWorldLocations[Index];

        const FVector TargetWorldLocation =
            SourceWorldLocation + SlotWidgetWorldOffset;

        FVector2D ScreenPosition;
        const bool bScreenProjected =
            PC->ProjectWorldLocationToScreen(
                TargetWorldLocation,
                ScreenPosition,
                true);

        FVector2D WidgetPosition;
        const bool bWidgetProjected =
            UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
                PC,
                TargetWorldLocation,
                WidgetPosition,
                true);

        UCanvasPanelSlot* CanvasSlot =
            Cast<UCanvasPanelSlot>(SlotWidget->Slot);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[ShopUI] Index=%d "
                "World=%s Target=%s "
                "ScreenOK=%s Screen=%s "
                "WidgetOK=%s Widget=%s "
                "Widget=%s Parent=%s SlotClass=%s"),
            Index,
            *SourceWorldLocation.ToString(),
            *TargetWorldLocation.ToString(),
            bScreenProjected ? TEXT("True") : TEXT("False"),
            *ScreenPosition.ToString(),
            bWidgetProjected ? TEXT("True") : TEXT("False"),
            *WidgetPosition.ToString(),
            *GetNameSafe(SlotWidget),
            *GetNameSafe(SlotWidget->GetParent()),
            SlotWidget->Slot
                ? *SlotWidget->Slot->GetClass()->GetName()
                : TEXT("None"));

        if (!CanvasSlot)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[ShopUI] CanvasSlot cast failed. "
                    "Index=%d SlotClass=%s"),
                Index,
                SlotWidget->Slot
                    ? *SlotWidget->Slot->GetClass()->GetName()
                    : TEXT("None"));
            continue;
        }

        const FGeometry ParentGeometry =
            SlotWidget->GetParent()
                ? SlotWidget->GetParent()->GetCachedGeometry()
                : FGeometry();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[ShopUI] Index=%d "
                "ParentAbsolutePos=%s ParentLocalSize=%s "
                "BeforePosition=%s Anchors Min=%s Max=%s Alignment=%s"),
            Index,
            *ParentGeometry.GetAbsolutePosition().ToString(),
            *ParentGeometry.GetLocalSize().ToString(),
            *CanvasSlot->GetPosition().ToString(),
            *CanvasSlot->GetAnchors().Minimum.ToString(),
            *CanvasSlot->GetAnchors().Maximum.ToString(),
            *CanvasSlot->GetAlignment().ToString());

        SlotWidget->SetVisibility(
            bWidgetProjected
                ? ESlateVisibility::Visible
                : ESlateVisibility::Collapsed);

        if (!bWidgetProjected)
        {
            continue;
        }

        CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        CanvasSlot->SetPosition(WidgetPosition);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShopUI] Index=%d AfterPosition=%s"),
            Index,
            *CanvasSlot->GetPosition().ToString());
    }
}

void UPBShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ShopSlotWidgets = 
	{
		Slot0,
		Slot1,
		Slot2,
		Slot3,
		Slot4,
		Slot5,
		Slot6,
		Slot7
	};
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this,
			&UPBShopWidget::OnExitButtonClicked);
	}
	
	if (Button_Reroll)
	{
		Button_Reroll->OnClicked.AddDynamic(this, &UPBShopWidget::OnRerollButtonClicked);
	}

	if (DeckWidget)
	{
		DeckWidget->OpenAll();
	}
	
	UpdateSlotWidgetPositionsOnce();
}



void UPBShopWidget::SetShopSlotWidgetData(TArray<const FBallDataStruct*> BallDatas)
{
	for(int i=0; i<BallDatas.Num(); i++)
	{
		
	}
	
}

void UPBShopWidget::SetShopSlotWidgetData(int32 index, FText Name, int32 Price, FText Synergy, UTexture2D* Icon)
{
	if (!ShopSlotWidgets.IsValidIndex(index) || !ShopSlotWidgets[index])
	{
		return;
	}

	ShopSlotWidgets[index]->SetVisibility(ESlateVisibility::Visible);
	ShopSlotWidgets[index]->SetSlotInfo(Name,Price,Synergy); 
	
}

void UPBShopWidget::UnActiveSlotWidget(int32 SlotIndex)
{
	if (!ShopSlotWidgets.IsValidIndex(SlotIndex) || !ShopSlotWidgets[SlotIndex])
	{
		return;
	}

	ShopSlotWidgets[SlotIndex]->SetVisibility(ESlateVisibility::Collapsed);
	ConfirmWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UPBShopWidget::RefuseWidgetSpawn()
{
	if (ConfirmWidget)
	{
		ConfirmWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if(RefuseWidget)
	{
		RefuseWidget->Show();
	}
}

void UPBShopWidget::SetPurchaseConfirmInfo(const FPBPurchaseConfirmData& Data)
{
	ConfirmWidget->SetInfo(Data);
	ConfirmWidget->SetVisibility(ESlateVisibility::Visible);
}

void UPBShopWidget::SetSlotWidgetLocation(const TArray<FVector>& InWorldLocations)
{
	ShopSlotWorldLocations = InWorldLocations;
	bPendingUpdateSlotPositions = true;
}

void UPBShopWidget::OnExitButtonClicked()
{
	FPBChoiceType Message;
	Message.Exit = 0;
	
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,Message);
	
	//RemoveFromParent();
}

void UPBShopWidget::OnRerollButtonClicked()
{
	OnRerollRequested.Broadcast();
}

void UPBShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bPendingUpdateSlotPositions)
	{
		bPendingUpdateSlotPositions = false;
		UpdateSlotWidgetPositionsOnce();
	}
}



