// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPurchaseConfirmWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include  "PinBallLike/Choice/UI/PBBettingWidget.h"
#include  "PinBallLike/Struct/Shop/PBPurChaseMessaage.h"
#include  "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "Kismet/GameplayStatics.h"
void UPBPurchaseConfirmWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBPurchaseConfirmWidget::OnConfirmButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBPurchaseConfirmWidget::OnCancelButtonClicked);
	}

	OpenAnimationFinishedEvent.BindDynamic(
		this,
		&UPBPurchaseConfirmWidget::OnOpenAnimationFinished);

	if (OpenAnim)
	{
		BindToAnimationFinished(
			OpenAnim,
			OpenAnimationFinishedEvent);
	}
}

void UPBPurchaseConfirmWidget::NativeDestruct()
{
	if (OpenAnim)
	{
		UnbindFromAnimationFinished(
			OpenAnim,
			OpenAnimationFinishedEvent);
	}

	Super::NativeDestruct();
}

void UPBPurchaseConfirmWidget::OnConfirmButtonClicked()
{
	FPBPurChaseMessaage Message;
	Message.SlotIndex = SlotIndex;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Shop_Purchase,
		Message);
	
	
}

void UPBPurchaseConfirmWidget::OnCancelButtonClicked()
{
	this->SetVisibility(ESlateVisibility::Hidden);
}

void UPBPurchaseConfirmWidget::OnOpenAnimationFinished()
{
	ConfirmButton->SetIsEnabled(true);
	CancelButton->SetIsEnabled(true);
}

void UPBPurchaseConfirmWidget::SetButtonsEnabled(bool bEnabled)
{

}

void UPBPurchaseConfirmWidget::SetInfo(
    const FPBPurchaseConfirmData& Data)
{
    SlotIndex = Data.SlotIndex;

    //------------------------------------
    // 이름
    //------------------------------------

    if (ItemNameText)
    {
        ItemNameText->SetText(Data.BallNameText);
    }

    //------------------------------------
    // 가격
    //------------------------------------

    if (PriceText)
    {
        PriceText->SetText(FText::Format(
            NSLOCTEXT("Shop", "Price", "{0} 골드"),
            FText::AsNumber(Data.Price)));
    }

    //------------------------------------
    // 아이콘
    //------------------------------------

    if (ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(Data.BallIconTexture);
    }

    //------------------------------------
    // 능력치
    //------------------------------------

    if (HPText)
    {
        HPText->SetText(Data.HpRow.ValueText);
    }

    if (MPText)
    {
        MPText->SetText(Data.MpRow.ValueText);
    }

    if (AttackText)
    {
        AttackText->SetText(Data.AttackRow.ValueText);
    }

    if (ManaRegenText)
    {
        ManaRegenText->SetText(Data.ManaRegenRow.ValueText);
    }

    //------------------------------------
    // 시너지(최대 3개)
    //------------------------------------

	//------------------------------------------------------
	// PowerFlip
	//------------------------------------------------------

	if (SynergyIcon1)
	{
		SynergyIcon1->SetBrushFromTexture(
			Data.PowerFlipData.IconTexture);
	}

	if (SynergyText1)
	{
		SynergyText1->SetText(
			Data.PowerFlipData.Text);
	}

	//------------------------------------------------------
	// Class
	//------------------------------------------------------

	if (SynergyIcon2)
	{
		SynergyIcon2->SetBrushFromTexture(
			Data.ClassData.IconTexture);
	}

	if (SynergyText2)
	{
		SynergyText2->SetText(
			Data.ClassData.Text);
	}

	//------------------------------------------------------
	// Race (첫 번째만 표시)
	//------------------------------------------------------

	SynergyIcon3->SetVisibility(ESlateVisibility::Hidden);
	SynergyText3->SetVisibility(ESlateVisibility::Hidden);
	
	if (Data.RaceDataList.IsValidIndex(0))
	{
		SynergyIcon3->SetVisibility(ESlateVisibility::Visible);
		SynergyText3->SetVisibility(ESlateVisibility::Visible);
		
		if (SynergyIcon3)
		{
			SynergyIcon3->SetBrushFromTexture(
				Data.RaceDataList[0].IconTexture);
		}

		if (SynergyText3)
		{
			SynergyText3->SetText(
				Data.RaceDataList[0].Text);
		}
	}
	
	SynergyIcon4->SetVisibility(ESlateVisibility::Hidden);
	SynergyText4->SetVisibility(ESlateVisibility::Hidden);
	
	if (Data.RaceDataList.IsValidIndex(1))
	{
		SynergyIcon4->SetVisibility(ESlateVisibility::Visible);
		SynergyText4->SetVisibility(ESlateVisibility::Visible);
		if (SynergyIcon4)
		{
			SynergyIcon4->SetBrushFromTexture(
				Data.RaceDataList[1].IconTexture);
		}

		if (SynergyText4)
		{
			SynergyText4->SetText(
				Data.RaceDataList[1].Text);
		}
		
	}
	
	
    if (SkillNameText)
    {
        SkillNameText->SetText(Data.SkillNameText);
    }
	
	if (SkillExplainText)
	{
		SkillExplainText->SetText(Data.SkillDescriptionText);
	}

    if (SkillIcon)
    {
        SkillIcon->SetBrushFromTexture(Data.SkillIconTexture);
    }
}


void UPBPurchaseConfirmWidget::PlayOpenAnimation()
{
	SetButtonsEnabled(false);

	if (!OpenAnim)
	{
		SetButtonsEnabled(true);
		return;
	}

	PlayAnimation(
		OpenAnim,
		0.f,
		1,
		EUMGSequencePlayMode::Forward,
		1.f);
}
